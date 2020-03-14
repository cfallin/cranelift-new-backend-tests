use regex::Regex;
use std::process::exit;

fn main() {
    println!("regex-rs: start");

    // Regex matching US-format phone numbers: (123) 456-7890
    let re = Regex::new(r"\(\d{3}\) \d{3}-\d{4}").unwrap();

    // Generate a test corpus.
    let mut tests = vec![];
    for i in 0..1000 {
        tests.push(format!(
            "({:03}) {:03}-{:04}",
            997 * i % 1000,
            787 * i % 1000,
            9973 * i % 10000
        ));
    }

    for _iter in 0..1000 {
        for t in &tests {
            if !re.is_match(t) {
                println!("BAD");
                exit(1);
            }
        }
    }

    println!("OK");
}
