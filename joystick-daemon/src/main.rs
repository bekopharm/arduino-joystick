use std::{env, error};

mod joystick;
mod serial;

fn main() -> Result<(), Box<dyn error::Error>> {
    let args: Vec<_> = env::args().collect();
    let port = if args.len() > 1 {
        args[1].clone()
    } else {
        "/dev/ttyUSB1".to_owned()
    };

    println!("Connecting to serial port at {}", port);
    let mut serial = serial::SerialConnection::new(&port.into(), 115200)?;

    let joystick = joystick::Joystick::new()?;

    println!(
        "Created joystick with device path {}",
        joystick.device_path()?.to_string_lossy()
    );

    loop {
        let button_state = serial.read_button_state()?;

        for (i, &pressed) in button_state.pressed.iter().enumerate() {
            joystick.button_press(button_map(i), pressed)?;
        }

        for (i, &value) in button_state.joysticks.iter().enumerate() {
            joystick.move_axis(axis_map(i), value as i32 - 512)?;
        }

        joystick.synchronise()?;
    }
}

fn button_map(i: usize) -> joystick::Button {
    use joystick::Button::*;
    match i {
        0 => LeftNorth,
        1 => LeftWest,
        2 => LeftEast,
        3 => LeftSouth,
        4 => LeftSpecial,
        5 => RightSouth,
        6 => RightSpecial,
        7 => RightEast,
        8 => RightWest,
        9 => RightNorth,
        10 => R2,
        11 => R1,
        12 => L2,
        13 => L1,
        14 => TriggerHappy1,
        15 => TriggerHappy2,
        16 => TriggerHappy3,
        17 => TriggerHappy4,
        18 => TriggerHappy5,
        19 => TriggerHappy6,
        20 => TriggerHappy7,
        21 => TriggerHappy8,
        22 => TriggerHappy9,
        23 => TriggerHappy10,
        24 => TriggerHappy11,
        25 => TriggerHappy12,
        26 => TriggerHappy13,
        27 => TriggerHappy14,
        28 => TriggerHappy15,
        29 => TriggerHappy16,
        30 => TriggerHappy17,
        31 => TriggerHappy18,
        32 => TriggerHappy19,
        33 => TriggerHappy20,
        34 => TriggerHappy21,
        35 => TriggerHappy22,
        36 => TriggerHappy23,
        37 => TriggerHappy24,
        38 => TriggerHappy25,
        39 => TriggerHappy26,
        40 => TriggerHappy27,
        41 => TriggerHappy28,
        42 => TriggerHappy29,
        43 => TriggerHappy30,
        44 => TriggerHappy31,
        45 => TriggerHappy32,
        46 => TriggerHappy33,
        47 => TriggerHappy34,
        _ => unreachable!(),
    }
}

fn axis_map(i: usize) -> joystick::Axis {
    use joystick::Axis::*;
    match i {
        0 => X,
        1 => Y,
        2 => RX,
        3 => RY,
        _ => unreachable!(),
    }
}
