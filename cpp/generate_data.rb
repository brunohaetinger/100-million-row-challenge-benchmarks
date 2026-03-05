#!/usr/bin/env ruby

require 'fileutils'

URLS = [
  'https://example.com/blog/scala-zio',
  'https://example.com/blog/functional-programming',
  'https://example.com/api/users',
  'https://example.com/api/products',
  'https://example.com/docs/getting-started',
  'https://example.com/docs/advanced',
  'https://example.com/',
  'https://example.com/contact',
  'https://example.com/about',
  'https://example.com/pricing'
].freeze

ROWS = ARGV[0]&.to_i || 100_000_000
FILE_PATH = 'measurements.txt'
BUFFER_SIZE = 8 * 1024 * 1024

random = Random.new(42)
start_date = Time.utc(2024, 1, 1, 0, 0)

puts "Generating #{ROWS} rows of test data..."

File.open(FILE_PATH, 'wb', BUFFER_SIZE) do |file|
  file.sync = false
  ROWS.times do |i|
    url = URLS[random.rand(URLS.length)]
    days_offset = random.rand(365)
    hours_offset = random.rand(24)
    minutes_offset = random.rand(60)
    seconds_offset = random.rand(60)

    timestamp = start_date + days_offset * 86_400 + hours_offset * 3600 + minutes_offset * 60 + seconds_offset
    formatted = timestamp.strftime('%Y-%m-%dT%H:%M:%S+00:00')

    file.write("#{url},#{formatted}\n")

    puts "Generated #{i + 1} rows..." if (i + 1) % 10_000_000 == 0
  end
end

puts "Generated #{ROWS} rows in #{FILE_PATH}"
