let api              = null;
let curImgHeight     = 0;

const processBtn     = document.getElementById('processBtn');
const fileInput      = document.getElementById('fileInput');
const algoSelect     = document.getElementById('algoSelect');
const rowHeightInput = document.getElementById('rowHeight');
const maxHeightLabel = document.getElementById('maxHeightLabel');
const warningBox     = document.getElementById('performanceWarning');
const qualitySlider  = document.getElementById('quality');
const qualityVal     = document.getElementById('qualityVal');
const spinner        = document.getElementById('spinner');
const statusText     = document.getElementById('statusText');
const resultImg      = document.getElementById('resultImg');
const downloadBtn    = document.getElementById('downloadBtn');

qualitySlider.addEventListener('input', (e) => {
    qualityVal.textContent = e.target.value;
});

function checkPerformanceWarning() {
    const algo       = algoSelect.value;
    const rowHeight  = parseInt(rowHeightInput.value) || 1;
    const isSlowAlgo = ['BUBBLE_SORT', 'INSERT_SORT', 'COMB_SORT', 'SHAKER_SORT'].includes(algo);

    if (isSlowAlgo && curImgHeight > 256 && rowHeight < 4)
        warningBox.style.display = 'block';
    else
        warningBox.style.display = 'none';
}

fileInput.addEventListener('change', (e) => {
    const file = e.target.files[0];
    if (!file) return;

    const reader   = new FileReader();
    reader.onload  = (event) => {
        const img  = new Image();
        img.onload = () => {
            curImgHeight         = img.height;
            rowHeightInput.max         = curImgHeight;
            maxHeightLabel.textContent = `max: ${curImgHeight}`;

            if (parseInt(rowHeightInput.value) > curImgHeight)
                rowHeightInput.value = curImgHeight;

            checkPerformanceWarning();
        };
        img.src = event.target.result;
    };
    reader.readAsDataURL(file);
});

algoSelect.addEventListener('change', checkPerformanceWarning);
rowHeightInput.addEventListener('input', checkPerformanceWarning);

// init WASM
WasmModule().then(module => {
    api                    = module;
    processBtn.textContent = 'Generate Animation';
    processBtn.disabled    = false;
    statusText.textContent = 'Select Image';
}).catch(err => {
    console.error("Failed to load WASM:", err);
    statusText.textContent = 'Error loading engine.';
});

processBtn.addEventListener('click', async () => {
    const file = fileInput.files[0];

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

            const algoString  = algoSelect.value;
            const algoEnum    = api.ALGORITHM[algoString];
            const rowHeight   = Math.min(parseInt(rowHeightInput.value), curImgHeight);
            const delay       = parseInt(document.getElementById('delay').value);
            const quality     = parseFloat(qualitySlider.value);
            const reverse     = false;

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
        }
        catch (error) {
            console.error("Processing error:", error);
            statusText.textContent = 'An error occurred during processing.';
            alert(error);
        }
        finally {
            // reset UI state
            spinner.style.display = 'none';
            processBtn.disabled   = false;
        }
    }, 50);
});
