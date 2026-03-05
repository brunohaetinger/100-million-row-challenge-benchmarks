#!/usr/bin/env bun

const URLS = [
  "https://example.com/blog/scala-zio",
  "https://example.com/blog/functional-programming",
  "https://example.com/api/users",
  "https://example.com/api/products",
  "https://example.com/docs/getting-started",
  "https://example.com/docs/advanced",
  "https://example.com/",
  "https://example.com/contact",
  "https://example.com/about",
  "https://example.com/pricing"
];

const ROWS = parseInt(Bun.argv[2]) || 100_000_000;
const FILE_PATH = "measurements.txt";

let seed = 42;
function seededRandom() {
  seed = (seed * 1103515245 + 12345) & 0x7fffffff;
  return seed / 0x7fffffff;
}

const random = {
  nextInt: (max) => Math.floor(seededRandom() * max),
};

const startDate = new Date("2024-01-01T00:00:00Z");

console.log(`Generating ${ROWS} rows of test data...`);

const file = Bun.file(FILE_PATH);
const writer = file.writer();

for (let i = 0; i < ROWS; i++) {
  const url = URLS[random.nextInt(URLS.length)];
  const daysOffset = random.nextInt(365);
  const hoursOffset = random.nextInt(24);
  const minutesOffset = random.nextInt(60);
  const secondsOffset = random.nextInt(60);

  const timestamp = new Date(startDate.getTime() + 
    daysOffset * 86400000 + 
    hoursOffset * 3600000 + 
    minutesOffset * 60000 + 
    secondsOffset * 1000
  );

  const formatted = timestamp.toISOString().replace("Z", "+00:00");
  
  writer.write(`${url},${formatted}\n`);

  if ((i + 1) % 10_000_000 === 0) {
    console.log(`Generated ${i + 1} rows...`);
  }
}

writer.end();
console.log(`Generated ${ROWS} rows in ${FILE_PATH}`);
