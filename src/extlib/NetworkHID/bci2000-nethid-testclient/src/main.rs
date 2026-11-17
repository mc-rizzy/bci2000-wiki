use std::{self, env, io, time::Duration};
use tokio::{io::AsyncWriteExt, net::TcpStream, time::sleep};
use anyhow::{Result, anyhow};

const KEYPRESS_WAIT_TIME: u64 = 50;

#[tokio::main]
async fn main() -> Result<()> {
    let args: Vec<String> = env::args().collect();
    if args.len() != 2 {
        return Err(anyhow!("provide a remote address in [addr]:[port] form"));
    }
    let remote_addr = args[1].clone();
    println!("Attempting to connect to {}", remote_addr);
    let mut conn = TcpStream::connect(remote_addr).await?;
    println!("connected!");

    let inp = io::stdin();
    loop {
        let mut msg = String::new();
        inp.read_line(&mut msg)?;
        if msg.starts_with("\\M") {
            let toks: Vec<&str> = msg.split(" ").collect();
            println!("{:?}", toks);
            kbd_move_mouse(&mut conn, toks[1].trim().parse()?, toks[2].trim().parse()?).await?;
        }
        else {
            println!("Send {}", msg);
            kbd_write_str(&mut conn, msg.trim()).await?;
        }
    }
    return Ok(());
}

async fn kbd_move_mouse(conn: &mut TcpStream, x: i32, y: i32) -> Result<()> {
    let message = Message::new(MessageType::MouseMove, x, y);
    conn.write_all(message.to_bytes().as_slice()).await?;
    println!("sent mouse move message");
    return Ok(());
}

async fn kbd_write_str(conn: &mut TcpStream, string: &str) -> Result<()> {
    for c in string.chars() {
        kbd_press_rel(conn, char_to_keycode(c)).await?;
    }
    return Ok(());
}

async fn kbd_press_rel(conn: &mut TcpStream, keycode: u32) -> Result<()> {
    kbd_press(conn, keycode).await?;
    sleep(Duration::from_millis(KEYPRESS_WAIT_TIME)).await;
    kbd_release(conn, keycode).await?;
    sleep(Duration::from_millis(KEYPRESS_WAIT_TIME)).await;
    return Ok(());
}

async fn kbd_press(conn: &mut TcpStream, keycode: u32) -> Result<()> {
    let message = Message::new(MessageType::KeyboardPress, keycode as i32, 0);
    conn.write_all(message.to_bytes().as_slice()).await?;
    println!("sent kbd press message");
    return Ok(());
}

async fn kbd_release(conn: &mut TcpStream, keycode: u32) -> Result<()> {
    let message = Message::new(MessageType::KeyboardRelease, keycode as i32, 0);
    conn.write_all(message.to_bytes().as_slice()).await?;
    return Ok(());
}

const ASCII_LOWERCASE_A: u8 = 0x61;
const KEYCODE_LOWERCASE_A: u8 = 0x04;
fn char_to_keycode(c: char) -> u32 {
    return (c as u8 - (ASCII_LOWERCASE_A - KEYCODE_LOWERCASE_A)) as u32;
}


#[derive(Clone, Copy)]
enum MessageType {
    KeyboardPress = 1,
    KeyboardRelease = 2,
    MouseMove = 3,
    MouseButton = 4
}

#[repr(C)]
struct Message {
    magic: u32,
    message_type: u32,
    message_arg1: i32,
    message_arg2: i32
}

impl Message {
    fn new(m_type: MessageType, arg1: i32, arg2: i32) -> Message {
        return Message {
            magic: 0x0bc12000,
            message_type: m_type as u32,
            message_arg1: arg1,
            message_arg2: arg2
        };
    }
    fn to_bytes(&self) -> Vec<u8> {
        let mut v: Vec<u8> = Vec::new();
        v.extend_from_slice(&self.magic.to_be_bytes());
        v.extend_from_slice(&self.message_type.to_be_bytes());
        v.extend_from_slice(&self.message_arg1.to_be_bytes());
        v.extend_from_slice(&self.message_arg2.to_be_bytes());
        return v;
    }
}


