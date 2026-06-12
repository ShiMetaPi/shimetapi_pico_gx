// rpi-detector Web UI Application
const MJPEG_URL = '/mjpeg';
const API_STATUS = '/api/status';
const API_MOTION_THRESHOLD = '/api/motion/threshold';
const API_ALERTS = '/api/alerts';
const API_MOTION_TRIGGER = '/api/motion/trigger';
const API_MOTION_RESET = '/api/motion/reset';

const ROI_STORAGE_KEY = 'rpi-detector-roi';
const DEFAULT_ROI = { x: 25, y: 25, w: 50, h: 50 };
const MIN_ROI_SIZE = 8;
const SAMPLE_WIDTH = 160;
const SAMPLE_HEIGHT = 120;
const PIXEL_DIFF_THRESHOLD = 42;
const DETECTION_INTERVAL_MS = 250;
const TRIGGER_COOLDOWN_MS = 1200;

let statusInterval;
let alertsInterval;
let streamRetryTimer;
let detectionTimer;
let motionThreshold = Number(localStorage.getItem('rpi-detector-threshold')) || 20;
let thresholdSyncTimer;
let roi = loadRoi();
let previousFrame = null;
let lastTriggerAt = 0;
let lastReportedCount = 0;
let roiDragState = null;

function clamp(value, min, max) {
    return Math.max(min, Math.min(max, value));
}

function loadRoi() {
    try {
        const saved = JSON.parse(localStorage.getItem(ROI_STORAGE_KEY));
        if (saved && Number.isFinite(saved.x) && Number.isFinite(saved.y) &&
            Number.isFinite(saved.w) && Number.isFinite(saved.h)) {
            return normalizeRoi(saved);
        }
    } catch (e) {
        console.warn('Failed to load ROI:', e);
    }
    return { ...DEFAULT_ROI };
}

function saveRoi() {
    localStorage.setItem(ROI_STORAGE_KEY, JSON.stringify(roi));
}

function normalizeRoi(next) {
    const normalized = { ...next };
    normalized.w = clamp(normalized.w, MIN_ROI_SIZE, 100);
    normalized.h = clamp(normalized.h, MIN_ROI_SIZE, 100);
    normalized.x = clamp(normalized.x, 0, 100 - normalized.w);
    normalized.y = clamp(normalized.y, 0, 100 - normalized.h);
    return normalized;
}

function applyRoi() {
    const box = document.getElementById('roi-box');
    if (!box) {
        return;
    }
    roi = normalizeRoi(roi);
    box.style.left = roi.x + '%';
    box.style.top = roi.y + '%';
    box.style.width = roi.w + '%';
    box.style.height = roi.h + '%';
}

function resetRoi() {
    roi = { ...DEFAULT_ROI };
    previousFrame = null;
    saveRoi();
    applyRoi();
}

function setStreamMessage(message, isError) {
    const overlay = document.getElementById('stream-overlay');
    if (!overlay) {
        return;
    }
    overlay.textContent = message || '';
    overlay.classList.toggle('hidden', !message);
    overlay.classList.toggle('error', !!isError);
}

function reloadStream() {
    const img = document.getElementById('mjpeg-stream');
    if (!img) {
        return;
    }

    clearTimeout(streamRetryTimer);
    previousFrame = null;
    setStreamMessage('正在连接实时画面...', false);
    img.src = MJPEG_URL + '?t=' + Date.now();
}

function scheduleStreamRetry() {
    clearTimeout(streamRetryTimer);
    streamRetryTimer = setTimeout(reloadStream, 3000);
}

async function fetchJson(url) {
    try {
        const resp = await fetch(url, { cache: 'no-store' });
        if (!resp.ok) {
            return null;
        }
        return await resp.json();
    } catch (e) {
        console.error('Fetch error:', e);
        return null;
    }
}

async function updateStatus() {
    const status = await fetchJson(API_STATUS);
    if (!status) {
        return;
    }

    const thresholdValue = document.getElementById('threshold-value');
    const motionCount = document.getElementById('motion-count');

    lastReportedCount = status.motion_count || lastReportedCount;

    if (thresholdValue) {
        thresholdValue.textContent = motionThreshold;
    }
    if (motionCount) {
        motionCount.textContent = lastReportedCount;
    }
}

function applyThresholdDisplay() {
    const thresholdValue = document.getElementById('threshold-value');
    const thresholdInput = document.getElementById('motion-threshold');

    if (thresholdValue) {
        thresholdValue.textContent = motionThreshold;
    }
    if (thresholdInput && Number(thresholdInput.value) !== motionThreshold) {
        thresholdInput.value = motionThreshold;
    }
}

function syncThresholdToBackend() {
    clearTimeout(thresholdSyncTimer);
    thresholdSyncTimer = setTimeout(async () => {
        try {
            await fetchJson(API_MOTION_THRESHOLD + '?val=' + motionThreshold);
        } catch (e) {
            console.error('Failed to set threshold:', e);
        }
    }, 150);
}

function setThreshold(val) {
    motionThreshold = clamp(parseInt(val, 10) || 1, 1, 80);
    previousFrame = null;
    localStorage.setItem('rpi-detector-threshold', String(motionThreshold));
    applyThresholdDisplay();
    syncThresholdToBackend();
}

async function updateAlerts() {
    const alerts = await fetchJson(API_ALERTS);
    if (!alerts) {
        return;
    }

    const alertsList = document.getElementById('alerts-list');
    if (alertsList) {
        alertsList.textContent = alerts.total > 0 ? `共 ${alerts.total} 次告警` : '无告警';
    }
}

async function resetCounters() {
    const result = await fetchJson(API_MOTION_RESET);
    lastReportedCount = result && Number.isFinite(result.motion_count) ? result.motion_count : 0;

    const motionCount = document.getElementById('motion-count');
    const alertsList = document.getElementById('alerts-list');
    if (motionCount) {
        motionCount.textContent = lastReportedCount;
    }
    if (alertsList) {
        alertsList.textContent = '无告警';
    }
    updateStatus();
    updateAlerts();
}

function getDisplayedImageRect(img) {
    const wrapRect = img.getBoundingClientRect();
    const naturalWidth = img.naturalWidth || SAMPLE_WIDTH;
    const naturalHeight = img.naturalHeight || SAMPLE_HEIGHT;
    const imgRatio = naturalWidth / naturalHeight;
    const wrapRatio = wrapRect.width / wrapRect.height;
    let width;
    let height;
    let left;
    let top;

    if (wrapRatio > imgRatio) {
        height = wrapRect.height;
        width = height * imgRatio;
        left = (wrapRect.width - width) / 2;
        top = 0;
    } else {
        width = wrapRect.width;
        height = width / imgRatio;
        left = 0;
        top = (wrapRect.height - height) / 2;
    }

    return { left, top, width, height, wrapWidth: wrapRect.width, wrapHeight: wrapRect.height };
}

function updateRoiFromPointer(event) {
    if (!roiDragState) {
        return;
    }

    const wrap = document.getElementById('stream-wrap');
    if (!wrap) {
        return;
    }

    const rect = wrap.getBoundingClientRect();
    const dx = ((event.clientX - roiDragState.startX) / rect.width) * 100;
    const dy = ((event.clientY - roiDragState.startY) / rect.height) * 100;
    const start = roiDragState.startRoi;
    const next = { ...start };

    if (roiDragState.handle === 'move') {
        next.x = start.x + dx;
        next.y = start.y + dy;
    } else {
        if (roiDragState.handle.includes('e')) {
            next.w = start.w + dx;
        }
        if (roiDragState.handle.includes('s')) {
            next.h = start.h + dy;
        }
        if (roiDragState.handle.includes('w')) {
            next.x = start.x + dx;
            next.w = start.w - dx;
        }
        if (roiDragState.handle.includes('n')) {
            next.y = start.y + dy;
            next.h = start.h - dy;
        }
    }

    roi = normalizeRoi(next);
    previousFrame = null;
    applyRoi();
}

function initRoiControls() {
    const box = document.getElementById('roi-box');
    if (!box) {
        return;
    }

    applyRoi();

    box.addEventListener('pointerdown', (event) => {
        event.preventDefault();
        const handle = event.target.dataset.handle || 'move';
        roiDragState = {
            handle,
            startX: event.clientX,
            startY: event.clientY,
            startRoi: { ...roi }
        };
        box.setPointerCapture(event.pointerId);
    });

    box.addEventListener('pointermove', updateRoiFromPointer);

    box.addEventListener('pointerup', (event) => {
        if (roiDragState) {
            updateRoiFromPointer(event);
            saveRoi();
        }
        roiDragState = null;
    });

    box.addEventListener('pointercancel', () => {
        roiDragState = null;
    });
}

function getRoiSampleRect(img) {
    const display = getDisplayedImageRect(img);
    const x = display.left + display.width * (roi.x / 100);
    const y = display.top + display.height * (roi.y / 100);
    const w = display.width * (roi.w / 100);
    const h = display.height * (roi.h / 100);

    return {
        sx: Math.max(0, Math.round((x - display.left) / display.width * SAMPLE_WIDTH)),
        sy: Math.max(0, Math.round((y - display.top) / display.height * SAMPLE_HEIGHT)),
        sw: Math.max(1, Math.round(w / display.width * SAMPLE_WIDTH)),
        sh: Math.max(1, Math.round(h / display.height * SAMPLE_HEIGHT))
    };
}

function markTriggered(active) {
    const box = document.getElementById('roi-box');
    if (!box) {
        return;
    }
    box.classList.toggle('triggered', active);
    if (active) {
        setTimeout(() => box.classList.remove('triggered'), 450);
    }
}

async function reportMotion(ratio) {
    const now = Date.now();
    if (now - lastTriggerAt < TRIGGER_COOLDOWN_MS) {
        return;
    }
    lastTriggerAt = now;
    markTriggered(true);

    const result = await fetchJson(API_MOTION_TRIGGER + '?ratio=' + ratio.toFixed(2));
    if (result && Number.isFinite(result.motion_count)) {
        lastReportedCount = result.motion_count;
        const motionCount = document.getElementById('motion-count');
        if (motionCount) {
            motionCount.textContent = lastReportedCount;
        }
    }
}

function detectMotionFrame() {
    const img = document.getElementById('mjpeg-stream');
    const canvas = document.getElementById('motion-canvas');
    const ratioEl = document.getElementById('motion-ratio');

    if (!img || !canvas || !img.complete || img.naturalWidth === 0 || img.naturalHeight === 0) {
        return;
    }

    canvas.width = SAMPLE_WIDTH;
    canvas.height = SAMPLE_HEIGHT;
    const ctx = canvas.getContext('2d', { willReadFrequently: true });
    if (!ctx) {
        return;
    }

    try {
        ctx.drawImage(img, 0, 0, SAMPLE_WIDTH, SAMPLE_HEIGHT);
    } catch (e) {
        return;
    }

    const frame = ctx.getImageData(0, 0, SAMPLE_WIDTH, SAMPLE_HEIGHT).data;
    const sampleRoi = getRoiSampleRect(img);

    if (!previousFrame) {
        previousFrame = new Uint8ClampedArray(frame);
        return;
    }

    let changed = 0;
    let total = 0;
    const maxY = Math.min(SAMPLE_HEIGHT, sampleRoi.sy + sampleRoi.sh);
    const maxX = Math.min(SAMPLE_WIDTH, sampleRoi.sx + sampleRoi.sw);

    for (let y = sampleRoi.sy; y < maxY; y += 1) {
        for (let x = sampleRoi.sx; x < maxX; x += 1) {
            const idx = (y * SAMPLE_WIDTH + x) * 4;
            const dr = Math.abs(frame[idx] - previousFrame[idx]);
            const dg = Math.abs(frame[idx + 1] - previousFrame[idx + 1]);
            const db = Math.abs(frame[idx + 2] - previousFrame[idx + 2]);
            if ((dr + dg + db) / 3 > PIXEL_DIFF_THRESHOLD) {
                changed += 1;
            }
            total += 1;
        }
    }

    const ratio = total > 0 ? (changed / total) * 100 : 0;
    previousFrame = new Uint8ClampedArray(frame);

    if (ratioEl) {
        ratioEl.textContent = ratio.toFixed(1);
    }

    if (ratio >= motionThreshold) {
        reportMotion(ratio);
    }
}

function initMotionDetection() {
    clearInterval(detectionTimer);
    detectionTimer = setInterval(detectMotionFrame, DETECTION_INTERVAL_MS);
}

function initStream() {
    const img = document.getElementById('mjpeg-stream');
    if (!img) {
        return;
    }

    setStreamMessage('正在连接实时画面...', false);

    img.addEventListener('load', () => {
        setStreamMessage('', false);
        previousFrame = null;
    });

    img.addEventListener('error', () => {
        setStreamMessage('实时画面连接失败，正在重试...', true);
        scheduleStreamRetry();
    });

    reloadStream();
}

function init() {
    motionThreshold = clamp(motionThreshold, 1, 80);
    applyThresholdDisplay();
    syncThresholdToBackend();
    initRoiControls();
    initStream();
    initMotionDetection();
    statusInterval = setInterval(updateStatus, 1000);
    alertsInterval = setInterval(updateAlerts, 5000);
    updateStatus();
    updateAlerts();
}

document.addEventListener('DOMContentLoaded', init);
