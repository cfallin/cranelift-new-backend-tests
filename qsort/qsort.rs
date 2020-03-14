/*
 * qsort test. Written 2020-03-13, Chris Fallin <cfallin@mozilla.com>.
 * Released into the public domain.
 */

use std::process::exit;

fn qsort(data: &mut [u32]) {
    if data.len() < 2 {
        return;
    }

    let pivot_i = data.len() - 1;
    let pivot = data[pivot_i];
    let mut i = 0;
    for j in 0..data.len() {
        if data[j] < pivot {
            let tmp = data[j];
            data[j] = data[i];
            data[i] = tmp;
            i += 1;
        }
    }

    let tmp = data[i];
    data[i] = data[pivot_i];
    data[pivot_i] = tmp;

    qsort(&mut data[..i]);
    qsort(&mut data[i + 1..]);
}

fn my_assert(cond: bool) {
    if !cond {
        println!("Fail");
        exit(1);
    }
}

fn main() {
    let mut data = vec![];
    for i in 0..100000 {
        data.push(4987 * i % 4919); // 4987, 4919 are primes
    }

    qsort(&mut data[..]);
    let mut last = 0;
    for x in &data {
        let x = *x;
        my_assert(x >= last);
        last = x;
    }

    println!("OK");
}
