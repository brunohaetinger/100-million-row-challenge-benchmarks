#!/usr/bin/env ruby

require 'yajl'
require 'fileutils'
require 'etc'

MIN_FILE_SIZE_FOR_PARALLEL = 1024 * 1024

def process_chunk_string(chunk_str)
  result = Hash.new { |h, k| h[k] = Hash.new(0) }

  chunk_str.each_line do |line|
    line = line.chomp
    next if line.empty?

    comma_idx = line.index(',')
    next unless comma_idx

    url = line[0, comma_idx]
    timestamp = line[comma_idx + 1, 10]

    path_start = url.index('/', 8)
    next unless path_start

    path = url[path_start..]

    result[path][timestamp] += 1
  end

  result.to_h.transform_values(&:to_h)
end

def process_file_sequential(file_path)
  result = Hash.new { |h, k| h[k] = Hash.new(0) }

  File.foreach(file_path) do |line|
    line = line.chomp
    next if line.empty?

    comma_idx = line.index(',')
    next unless comma_idx

    url = line[0, comma_idx]
    timestamp = line[comma_idx + 1, 10]

    path_start = url.index('/', 8)
    next unless path_start

    path = url[path_start..]

    result[path][timestamp] += 1
  end

  result
end

def format_output(aggregated)
  grouped = Hash.new { |h, k| h[k] = {} }

  aggregated.each do |path, dates|
    sorted_dates = dates.keys.sort.each_with_object({}) { |date, h| h[date] = dates[date] }
    grouped[path] = sorted_dates
  end

  grouped.keys.sort.each_with_object({}) { |path, h| h[path] = grouped[path] }
end

def split_file_into_chunks(input_file, num_chunks)
  file_size = File.size(input_file)
  chunk_size = file_size / num_chunks

  return [File.read(input_file)] if chunk_size < MIN_FILE_SIZE_FOR_PARALLEL

  chunks = []

  File.open(input_file, 'rb') do |file|
    num_chunks.times do |i|
      start_pos = i * chunk_size
      end_pos = i == num_chunks - 1 ? file_size : (i + 1) * chunk_size

      file.pos = start_pos
      file.gets if i > 0

      actual_start = file.pos
      remaining = end_pos - actual_start

      if remaining > 0
        chunk_data = file.read(remaining)
        chunks << chunk_data
      end
    end
  end

  chunks
end

input_file = ARGV[0] || 'measurements.txt'

unless File.exist?(input_file)
  puts "Error: Input file '#{input_file}' not found"
  exit 1
end

num_workers = ENV['WORKERS']&.to_i || Etc.nprocessors
file_size = File.size(input_file)

puts "Processing file: #{input_file}"
puts "File size: #{file_size / (1024 * 1024)} MB"
puts "Workers: #{num_workers}"

start_time = Process.clock_gettime(Process::CLOCK_MONOTONIC)

chunks = split_file_into_chunks(input_file, num_workers)

if chunks.size == 1
  result = process_file_sequential(input_file)
else
  pids = chunks.map do |chunk|
    read_io, write_io = IO.pipe

    pid = fork do
      read_io.close
      result = process_chunk_string(chunk)
      write_io.write(Marshal.dump(result))
      write_io.close
      exit!
    end

    write_io.close
    [pid, read_io]
  end

  results = pids.map do |pid, read_io|
    result = Marshal.load(read_io.read)
    read_io.close
    Process.wait(pid)
    result
  end

  result = results.reduce do |acc, r|
    r.each do |path, dates|
      dates.each do |date, count|
        acc[path][date] += count
      end
    end
    acc
  end
end

output = format_output(result)

json_output = Yajl::Encoder.encode(output, pretty: true)
File.write('output.json', json_output)

end_time = Process.clock_gettime(Process::CLOCK_MONOTONIC)
duration = end_time - start_time

puts "Processed in #{duration.round(3)} seconds"
puts 'Output written to: output.json'
