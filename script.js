let api             = null;

const processBtn    = document.getElementById('processBtn');
const qualitySlider = document.getElementById('quality');
const qualityVal    = document.getElementById('qualityVal');
const spinner       = document.getElementById('spinner');
const statusText    = document.getElementById('statusText');
const resultImg     = document.getElementById('resultImg');
const downloadBtn   = document.getElementById('downloadBtn');

qualitySlider.addEventListener('input', (e) => {
    qualityVal.textContent = e.target.value;
});

// init WASM
WasmModule().then(module => {
    api = module;
    processBtn.textContent = 'Generate Animation';
    processBtn.disabled = false;
    statusText.textContent = 'Ready.';
}).catch(err => {
    console.error("Failed to load WASM:", err);
    statusText.textContent = 'Error loading engine.';
});

processBtn.addEventListener('click', async () => {
    const file = document.getElementById('fileInput').files[0];

    if (!file) {
        alert("Please select an image first!");
        return;
    }

    processBtn.disabled       = true;
    resultImg.style.display   = 'none';
    downloadBtn.style.display = 'none';
    spinner.style.display     = 'block';
    statusText.textContent    = 'Processing... (this may take a moment)';

    setTimeout(async () => {
        try {
            const arrayBuffer = await file.arrayBuffer();
            const uint8Array  = new Uint8Array(arrayBuffer);

            const image       = api.createImage(uint8Array);

            const algoString  = document.getElementById('algoSelect').value;
            const algoEnum    = api.ALGORITHM[algoString];
            const rowHeight   = parseInt(document.getElementById('rowHeight').value);
            const delay       = parseInt(document.getElementById('delay').value);
            const quality     = parseFloat(document.getElementById('quality').value);
            const reverse     = document.getElementById('reverse').checked;

            image.setParams(algoEnum, rowHeight, delay, quality, reverse);
            const webpData    = image.generateWebP();

            image.delete();

            const blob        = new Blob([webpData], { type: 'image/webp' });
            const url         = URL.createObjectURL(blob);

            resultImg.src     = url;
            downloadBtn.href  = url;

            resultImg.style.display   = 'block';
            downloadBtn.style.display = 'block';
            statusText.textContent    = 'Done!';

        } catch (error) {
            console.error("Processing error:", error);
            statusText.textContent = 'An error occurred during processing.';
            alert(error);
        } finally {
            // reset UI state
            spinner.style.display = 'none';
            processBtn.disabled   = false;
        }
    }, 50);
});
