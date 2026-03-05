#!/usr/bin/env bun

const inputFile = Bun.argv[2] || "measurements.txt";

const file = Bun.file(inputFile);

if (!await file.exists()) {
  console.error(`Error: Input file '${inputFile}' not found`);
  process.exit(1);
}

const fileSize = file.size;
console.log(`Processing file: ${inputFile}`);
console.log(`File size: ${Math.floor(fileSize / (1024 * 1024))} MB`);

const startTime = performance.now();

const result = {};

const reader = await file.stream().getReader();
const decoder = new TextDecoder("utf-8", { fatal: false });
let buffer = "";

while (true) {
  const { done, value } = await reader.read();
  
  if (done) {
    if (buffer.length > 0) {
      const lines = buffer.split("\n");
      for (const line of lines) {
        if (!line) continue;
        
        const commaIdx = line.indexOf(",");
        if (commaIdx === -1) continue;
        
        const pathStart = line.indexOf("/", 8);
        if (pathStart === -1) continue;
        
        const path = line.slice(pathStart, commaIdx);
        const timestamp = line.slice(commaIdx + 1, commaIdx + 11);
        
        if (result[path]) {
          result[path][timestamp] = (result[path][timestamp] || 0) + 1;
        } else {
          result[path] = { [timestamp]: 1 };
        }
      }
    }
    break;
  }
  
  buffer += decoder.decode(value, { stream: true });
  
  const lastNewline = buffer.lastIndexOf("\n");
  if (lastNewline === -1) continue;
  
  const chunk = buffer.slice(0, lastNewline);
  buffer = buffer.slice(lastNewline + 1);
  
  const lines = chunk.split("\n");
  for (const line of lines) {
    if (!line) continue;
    
    const commaIdx = line.indexOf(",");
    if (commaIdx === -1) continue;
    
    const pathStart = line.indexOf("/", 8);
    if (pathStart === -1) continue;
    
    const path = line.slice(pathStart, commaIdx);
    const timestamp = line.slice(commaIdx + 1, commaIdx + 11);
    
    if (result[path]) {
      result[path][timestamp] = (result[path][timestamp] || 0) + 1;
    } else {
      result[path] = { [timestamp]: 1 };
    }
  }
}

const grouped = {};
const paths = Object.keys(result).sort();

for (const path of paths) {
  const dates = result[path];
  const sortedDates = Object.keys(dates).sort().reduce((acc, date) => {
    acc[date] = dates[date];
    return acc;
  }, {});
  grouped[path] = sortedDates;
}

const jsonOutput = JSON.stringify(grouped);
await Bun.write("output.json", jsonOutput);

const endTime = performance.now();
const duration = (endTime - startTime) / 1000;

console.log(`Processed in ${duration.toFixed(3)} seconds`);
console.log("Output written to: output.json");
