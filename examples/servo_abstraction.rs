#![no_std]
#![no_main]

use panic_halt as _;
use arduino_hal::prelude::*;

pub struct Servo<'a> {
    tc1: &'a arduino_hal::pac::TC1,
}

impl<'a> Servo<'a> {
    pub fn new(tc1: &'a arduino_hal::pac::TC1) -> Self {
        // Set up TC1 for servo control
        tc1.icr1.write(|w| w.bits(4999));
        tc1.tccr1a
            .write(|w| w.wgm1().bits(0b10).com1a().match_clear());
        tc1.tccr1b
            .write(|w| w.wgm1().bits(0b11).cs1().prescale_64());

        Self { tc1 }
    }

    pub fn write(&self, position: u16) {
        let duty = 100 + (position as u16 * 6); // Simple map position (0-100) to duty cycle
        self.tc1.ocr1a.write(|w| w.bits(duty));
    }
}

#[arduino_hal::entry]
fn main() -> ! {
    let dp = arduino_hal::Peripherals::take().unwrap();
    let pins = arduino_hal::pins!(dp);

    pins.d9.into_output();

    let servo = Servo::new(&dp.TC1);

    loop {
        // Sweep from 0 to 100 (0.4ms to 2.8ms)
        // for pos in 0..=100 {
        //     servo.write(pos);
        //     arduino_hal::delay_ms(20);
        // }
        // Sweep back from 100 to 0
        // for pos in (0..=100).rev() {
        //     servo.write(pos);
        //     arduino_hal::delay_ms(20);
        // }
        servo.write(20);
        arduino_hal::delay_ms(1000);
        servo.write(80);
        arduino_hal::delay_ms(1000);
    }
}
