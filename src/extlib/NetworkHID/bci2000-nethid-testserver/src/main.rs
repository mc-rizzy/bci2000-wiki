use anyhow::Result;
use tokio::{io::AsyncReadExt, net::TcpListener};

#[tokio::main]
async fn main() -> Result<()> {
    let addr = "127.0.0.1:4559";
    let listener = TcpListener::bind(addr).await?;
    println!("Running on {}", addr);

    loop {
        let (mut socket, _) = listener.accept().await?;
        loop {
            let mut buf: Vec<u8> = vec![0;1024];
            let in_bytes = socket.read(&mut buf).await?;

            if in_bytes == 0 {
                println!("Connection closed");
                break;
            }

            let mut index: usize = 0;
            loop {
                let magic = u32::from_be_bytes(buf[index..index+4].try_into().unwrap());
                let message_type = u32::from_be_bytes(buf[index+4..index+8].try_into().unwrap());
                let arg1 = i32::from_be_bytes(buf[index+8..index+12].try_into().unwrap());
                let arg2 = i32::from_be_bytes(buf[index+12..index+16].try_into().unwrap());
                if magic == 0 {
                    println!("Recv end of message");
                    break;
                } else {
                    match message_type {
                        1 => {
                            println!("Keyboard press keycode {}", arg1)
                        },
                        2 => {
                            println!("Keyboard release keycode {}", arg1)
                        },
                        3 => {
                            println!("Mouse move x:{} y:{}", arg1, arg2)
                        }
                        4 => {
                            println!("Mouse button left {} right {}", arg1, arg2);
                        },
                        i => {
                            println!("Received invalid message type {}", i)
                        }
                    }
                }
                index += 16;
            }
        }
    }
}
