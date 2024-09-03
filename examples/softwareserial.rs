#![no_std]
#![no_main]
#![feature(abi_avr_interrupt)]

extern crate panic_halt;

use arduino_hal::hal::port::{Pin};
use arduino_hal::prelude::*;
use arduino_hal::port::mode::{Floating, Input, Output, PullUp};
use core::cell::RefCell;
use core::sync::atomic::{AtomicU8, AtomicBool, Ordering};
use avr_device::interrupt::{self, Mutex};

// Define constants
const SS_MAX_RX_BUFF: usize = 64;

// Static variables for the ISR to access
static RX_BUFFER: Mutex<RefCell<[u8; SS_MAX_RX_BUFF]>> = Mutex::new(RefCell::new([0; SS_MAX_RX_BUFF]));
static RX_BUFFER_HEAD: AtomicU8 = AtomicU8::new(0);
static RX_BUFFER_TAIL: AtomicU8 = AtomicU8::new(0);
static BUFFER_OVERFLOW: AtomicBool = AtomicBool::new(false);

pub struct SoftwareSerial {
    rx: Pin<Input<PullUp>>,
    tx: Pin<Output>,
    rx_delay_centering: u16,
    rx_delay_intrabit: u16,
    rx_delay_stopbit: u16,
    tx_delay: u16,
}

impl SoftwareSerial {
    pub fn new(rx: Pin<Input<PullUp>>, tx: Pin<Output>) -> SoftwareSerial {
        SoftwareSerial {
            rx,
            tx,
            rx_delay_centering: 0,
            rx_delay_intrabit: 0,
            rx_delay_stopbit: 0,
            tx_delay: 0,
        }
    }

    pub fn begin(&mut self, baud_rate: u32) {
        // Calculate the delays based on the baud rate and CPU frequency
        let bit_delay = (arduino_hal::clock::cpu_freq_hz() / baud_rate) / 4;
        self.tx_delay = bit_delay as u16 - 15 / 4;
        self.rx_delay_centering = (bit_delay / 2) as u16 - 23 / 4;
        self.rx_delay_intrabit = bit_delay as u16 - 23 / 4;
        self.rx_delay_stopbit = (bit_delay * 3 / 4) as u16 - 23 / 4;

        // Enable pin change interrupt for the RX pin
        self.enable_interrupt();
    }

    fn enable_interrupt(&mut self) {
        // Enable pin change interrupt on the RX pin
        let pcicr = unsafe { &*arduino_hal::pac::PCICR::ptr() };
        let pcmsk0 = unsafe { &*arduino_hal::pac::PCMSK0::ptr() };
        
        // Assuming the RX pin is on PORTB (corresponds to PCINT0)
        pcicr.write(|w| w.pcint0().set_bit());
        pcmsk0.write(|w| w.bits(self.rx.deref().mask()));
    }

    pub fn available(&self) -> u8 {
        let head = RX_BUFFER_HEAD.load(Ordering::Relaxed);
        let tail = RX_BUFFER_TAIL.load(Ordering::Relaxed);
        (head.wrapping_sub(tail)) % SS_MAX_RX_BUFF as u8
    }

    pub fn read(&self) -> Option<u8> {
        let head = RX_BUFFER_HEAD.load(Ordering::Relaxed);
        let tail = RX_BUFFER_TAIL.load(Ordering::Relaxed);

        if head == tail {
            return None; // Buffer empty
        }

        let data = interrupt::free(|cs| {
            let mut buffer = RX_BUFFER.borrow(cs).borrow_mut();
            let data = buffer[tail as usize];
            RX_BUFFER_TAIL.store((tail.wrapping_add(1)) % SS_MAX_RX_BUFF as u8, Ordering::Relaxed);
            data
        });

        Some(data)
    }

    fn tuned_delay(&self, delay: u16) {
        arduino_hal::simple_delay::DelayUs::delay_us(delay as u32);
    }

    fn recv(&mut self) {
        let mut d: u8 = 0;

        if !self.rx.is_high() {
            self.tuned_delay(self.rx_delay_centering);

            for _ in 0..8 {
                self.tuned_delay(self.rx_delay_intrabit);
                d >>= 1;
                if self.rx.is_high() {
                    d |= 0x80;
                }
            }

            let head = RX_BUFFER_HEAD.load(Ordering::Relaxed);
            let next_head = (head + 1) % SS_MAX_RX_BUFF as u8;

            if next_head != RX_BUFFER_TAIL.load(Ordering::Relaxed) {
                interrupt::free(|cs| {
                    let mut buffer = RX_BUFFER.borrow(cs).borrow_mut();
                    buffer[head as usize] = d;
                });
                RX_BUFFER_HEAD.store(next_head, Ordering::Relaxed);
            } else {
                BUFFER_OVERFLOW.store(true, Ordering::Relaxed);
            }

            self.tuned_delay(self.rx_delay_stopbit);
        }
    }

    fn handle_interrupt(&mut self) {
        self.recv();
    }
}

// Interrupt Service Routine for Pin Change Interrupt on PCINT0 (usually PORTB)
#[avr_device::interrupt(atmega328p::PCINT0)]
fn pin_change_interrupt() {
    let mut serial = SOFTWARE_SERIAL.lock().unwrap();
    serial.handle_interrupt();
}

// Global variable to hold the SoftwareSerial instance
static SOFTWARE_SERIAL: Mutex<RefCell<Option<SoftwareSerial>>> = Mutex::new(RefCell::new(None));

#[arduino_hal::entry]
fn main() -> ! {
    let dp = arduino_hal::Peripherals::take().unwrap();
    let pins = arduino_hal::pins!(dp);
    let mut serial = arduino_hal::default_serial!(dp, pins, 57600);

    let rx_pin = pins.d2.into_pull_up_input(); // RX pin (PCINT0)
    let tx_pin = pins.d3.into_output();        // TX pin

    // Initialize the SoftwareSerial instance
    interrupt::free(|cs| {
        let mut ss = SOFTWARE_SERIAL.borrow(cs).borrow_mut();
        *ss = Some(SoftwareSerial::new(rx_pin, tx_pin));
        if let Some(ref mut ss) = *ss {
            ss.begin(9600);
        }
    });

    // Enable interrupts globally
    unsafe { avr_device::interrupt::enable() };

    loop {
        if let Some(data) = interrupt::free(|cs| {
            let ss = SOFTWARE_SERIAL.borrow(cs).borrow();
            ss.as_ref().and_then(|ss| ss.read())
        }) {
            ufmt::uwriteln!(&mut serial, "Received: {}\r", data).unwrap();
        }
    }
}
