#![no_std]
#![no_main]


use cyw43_pio::{PioSpi, DEFAULT_CLOCK_DIVIDER};

use defmt::*;

use embassy_executor::Spawner;
use embassy_rp::bind_interrupts;
use embassy_rp::block::ImageDef;
use embassy_rp::gpio::{Level, Output};
use embassy_rp::peripherals::{DMA_CH0, PIO0};
use embassy_rp::pio::{InterruptHandler, Pio};
use embassy_time::{Duration, Timer};

use static_cell::StaticCell;
use {defmt_rtt as _, panic_probe as _};


#[unsafe(link_section = ".start_block")]
#[used]
pub static IMAGE_DEF: ImageDef = ImageDef::secure_exe();

// Program metadata for `picotool info`.
// This isn't needed, but it's recommended to have these minimal entries.
#[unsafe(link_section = ".bi_entries")]
#[used]
pub static PICOTOOL_ENTRIES: [embassy_rp::binary_info::EntryAddr; 4] = [
    embassy_rp::binary_info::rp_program_name!(c"Embassy Template w/ WiFi & BT"),
    embassy_rp::binary_info::rp_program_description!(
        c"This example tests the Pico2W w/ the embassy libraries. WiFi & BT are tested as well as the SYS_LED"
    ),
    embassy_rp::binary_info::rp_cargo_version!(),
    embassy_rp::binary_info::rp_program_build_attribute!(),
];

const WIFI_FIRMWARE_BASE: u32 = 0x1030_0000;
const BT_FIRMWARE_BASE: u32 = 0x1034_0000;
const CLM_FIRMWARE_BASE: u32 = 0x1034_4000;

bind_interrupts!(struct Irqs {
    PIO0_IRQ_0 => InterruptHandler<PIO0>;
});

#[embassy_executor::task]
async fn cyw43_task(
    runner: cyw43::Runner<'static, Output<'static>, PioSpi<'static, PIO0, 0, DMA_CH0>>,
) -> ! {
    runner.run().await
}

#[embassy_executor::main]
async fn main(spawner: Spawner) {
    let p = embassy_rp::init(Default::default());
    let fw = unsafe { core::slice::from_raw_parts(WIFI_FIRMWARE_BASE as *const u8, 231077) };
    let btfw = unsafe { core::slice::from_raw_parts(BT_FIRMWARE_BASE as *const u8, 6164) };
    let clm = unsafe { core::slice::from_raw_parts(CLM_FIRMWARE_BASE as *const u8, 984) };

    let pwr = Output::new(p.PIN_23, Level::Low);
    let cs = Output::new(p.PIN_25, Level::High);
    let mut pio = Pio::new(p.PIO0, Irqs);
    let spi = PioSpi::new(
        &mut pio.common,
        pio.sm0,
        DEFAULT_CLOCK_DIVIDER,
        pio.irq0,
        cs,
        p.PIN_24,
        p.PIN_29,
        p.DMA_CH0,
    );

    static STATE: StaticCell<cyw43::State> = StaticCell::new();
    let state = STATE.init(cyw43::State::new());
    let (_net_device, _bt_device, mut control, runner) =
        cyw43::new_with_bluetooth(state, pwr, spi, fw, btfw).await;
    unwrap!(spawner.spawn(cyw43_task(runner)));

    control.init(clm).await;
    control
        .set_power_management(cyw43::PowerManagementMode::PowerSave)
        .await;


    // blink example
    let loop_delay = Duration::from_secs(5);
    let blink_delay = Duration::from_millis(125);
    loop {
        info!("All done - Waiting in loop!");
        for _ in 0..4 {
            control.gpio_set(0, true).await;
            Timer::after(blink_delay).await;
            control.gpio_set(0, false).await;
            Timer::after(blink_delay).await;
        }
        Timer::after(loop_delay).await;
    }
}
