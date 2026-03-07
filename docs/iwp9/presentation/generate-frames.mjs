#!/usr/bin/env node
// Generate transparent PNG screenshots of each presentation frame for Prezi
import puppeteer from 'puppeteer';
import { fileURLToPath } from 'url';
import { dirname, join } from 'path';
import { mkdirSync } from 'fs';

const __dirname = dirname(fileURLToPath(import.meta.url));
const outDir = join(__dirname, 'frames');
mkdirSync(outDir, { recursive: true });

const htmlPath = `file://${join(__dirname, 'presentation-transparent-for-prezi.html')}`;

async function main() {
    const browser = await puppeteer.launch({
        headless: true,
        executablePath: '/Applications/Google Chrome.app/Contents/MacOS/Google Chrome',
    });
    const page = await browser.newPage();

    // High resolution for crisp text in Prezi
    await page.setViewport({ width: 1200, height: 2000, deviceScaleFactor: 2 });

    await page.goto(htmlPath, { waitUntil: 'networkidle0' });

    // Get all frame IDs
    const frameIds = await page.evaluate(() => {
        return Array.from(document.querySelectorAll('.frame')).map(el => el.id);
    });

    console.log(`Found ${frameIds.length} frames to capture`);

    for (const id of frameIds) {
        const element = await page.$(`#${id}`);
        if (!element) {
            console.log(`  Skipping ${id} — not found`);
            continue;
        }

        // Make body background transparent for true PNG transparency
        await page.evaluate(() => {
            document.body.style.background = 'transparent';
        });

        const filename = id.replace('frame-', '') + '.png';
        await element.screenshot({
            path: join(outDir, filename),
            omitBackground: true,  // key: transparent background
        });

        console.log(`  Captured ${filename}`);
    }

    await browser.close();
    console.log(`\nDone! ${frameIds.length} frames saved to ${outDir}/`);
}

main().catch(err => { console.error(err); process.exit(1); });
