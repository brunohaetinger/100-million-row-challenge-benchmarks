use clap::Parser;
use memmap2::Mmap;
use rayon::prelude::*;
use serde::{Deserialize, Serialize};
use std::collections::HashMap;
use std::fs::File;
use std::time::Instant;

#[derive(Parser, Debug)]
#[command(author, version, about, long_about = None)]
struct Args {
    #[arg(default_value = "measurements.txt")]
    input_file: String,
}

#[derive(Serialize, Deserialize, Clone, Default)]
struct AggregatedData {
    #[serde(flatten)]
    data: HashMap<String, HashMap<String, u64>>,
}

fn find_path_and_date(line: &str) -> Option<(String, String)> {
    let comma_pos = line.find(',')?;
    let url = &line[..comma_pos];
    let timestamp = &line[comma_pos + 1..std::cmp::min(comma_pos + 1 + 10, line.len())];

    let path_start = url.find('/').filter(|&i| i >= 8)?;
    let path = &url[path_start..comma_pos];

    Some((path.to_string(), timestamp.to_string()))
}

fn process_chunk(file_data: &[u8], start: usize, end: usize) -> AggregatedData {
    let mut result = AggregatedData::default();

    let mut pos = start;
    while pos < end {
        let line_end = file_data[pos..end]
            .iter()
            .position(|&b| b == b'\n')
            .map(|i| pos + i)
            .unwrap_or(end);

        if line_end == pos {
            pos = line_end + 1;
            continue;
        }

        let line_slice = &file_data[pos..line_end];
        let line_str = std::str::from_utf8(line_slice).unwrap_or("");

        if let Some((path, date)) = find_path_and_date(line_str) {
            *result
                .data
                .entry(path.to_string())
                .or_insert_with(HashMap::new)
                .entry(date.to_string())
                .or_insert(0) += 1;
        }

        pos = line_end + 1;
    }

    result
}

fn format_output(mut data: AggregatedData) -> AggregatedData {
    let paths: Vec<_> = data.data.keys().cloned().collect();

    for path in paths {
        let dates_map = data.data.remove(&path).unwrap();
        let mut dates: Vec<_> = dates_map.keys().cloned().collect();
        dates.sort();

        let mut sorted_dates = HashMap::new();
        for date in dates {
            sorted_dates.insert(date.clone(), dates_map[&date]);
        }

        data.data.insert(path, sorted_dates);
    }

    data
}

fn main() {
    let args = Args::parse();

    let file_size = std::fs::metadata(&args.input_file)
        .map(|m| m.len())
        .unwrap_or(0);

    println!("Processing file: {}", args.input_file);
    println!("File size: {} MB", file_size as f64 / 1_000_000.0);

    let start_time = Instant::now();

    let file = File::open(&args.input_file).expect("Cannot open file");
    let mmap = unsafe { Mmap::map(&file).expect("mmap failed") };

    let num_workers = rayon::current_num_threads();

    let chunk_size = (file_size as usize) / num_workers;

    let chunks: Vec<_> = (0..num_workers as usize)
        .map(|i| {
            let start = i * chunk_size;
            let end = if i == num_workers as usize - 1 {
                mmap.len()
            } else {
                (i + 1) * chunk_size
            };
            (start, end)
        })
        .collect();

    let results: Vec<_> = chunks
        .par_iter()
        .map(|&(start, end)| process_chunk(&mmap[start..end], 0, end - start))
        .collect();

    let mut merged = AggregatedData::default();
    for res in results {
        for (path, dates) in res.data {
            let path_map = merged.data.entry(path).or_insert_with(HashMap::new);
            for (date, count) in dates {
                *path_map.entry(date).or_insert(0) += count;
            }
        }
    }

    let output = format_output(merged);

    let json = serde_json::to_string(&output).expect("JSON serialization failed");
    std::fs::write("output.json", json.as_bytes()).expect("Cannot write output");

    let duration = start_time.elapsed();
    println!("Processed in {:.3} seconds", duration.as_secs_f64());
    println!("Output written to: output.json");
}
