const TESTS = [
  { item: "rtc", name: "RTC 时钟", url: "/api/test/rtc" },
  { item: "spi_flash", name: "SPI Flash", url: "/api/test/spi_flash" },
  { item: "sdcard", name: "SD 卡读写", url: "/api/test/sdcard" },
  { item: "sensor", name: "Sensor 出图", url: "/api/test/sensor" },
  { item: "mjpeg", name: "MJPEG 图传", url: "/api/test/mjpeg" },
  { item: "led", name: "LED", url: "/api/test/led" },
];

const STATUS_TEXT = {
  IDLE: "未测试",
  RUNNING: "测试中",
  PASS: "PASS",
  FAIL: "FAIL",
  MANUAL: "人工确认",
  SKIP: "跳过",
};

let testResults = {};
let ledModalResolver = null;
let previewStarted = false;

const END_TEST_RESULT_FILE = "/tmp/factory_test_result.json";

document.addEventListener("DOMContentLoaded", () => {
  document.getElementById("boardAddress").textContent = window.location.host || "-";

  updatePageTime();
  setInterval(updatePageTime, 1000);

  document.getElementById("runAllBtn").addEventListener("click", runAllTests);
  document.getElementById("clearBtn").addEventListener("click", clearResults);
  document.getElementById("endTestBtn").addEventListener("click", endTest);

  startPreview();

  document.querySelectorAll("[data-test]").forEach((button) => {
    button.addEventListener("click", () => runSingleTest(button.getAttribute("data-test")));
  });

  document.getElementById("ledYesBtn").addEventListener("click", () => resolveLedConfirm(true));
  document.getElementById("ledNoBtn").addEventListener("click", () => resolveLedConfirm(false));

  loadServerInfo();
  clearResults();
});

async function loadServerInfo() {
  try {
    const info = await requestJson("/api/info");
    document.getElementById("serverVersion").textContent = info.version || "-";
    appendLog("Factory server ready: " + (info.app || "factory_server"));
  } catch (error) {
    document.getElementById("serverVersion").textContent = "连接失败";
    appendLog("读取服务信息失败：" + error.message);
  }
}

function updatePageTime() {
  document.getElementById("pageTime").textContent = formatDateTime(new Date());
}

function formatDateTime(date) {
  const pad = (value) => String(value).padStart(2, "0");

  return date.getFullYear()
    + "-" + pad(date.getMonth() + 1)
    + "-" + pad(date.getDate())
    + " " + pad(date.getHours())
    + ":" + pad(date.getMinutes())
    + ":" + pad(date.getSeconds());
}

async function runAllTests() {
  setButtonsDisabled(true);
  clearResults();
  setOverall("RUNNING");
  appendLog("开始一键测试");

  for (const test of TESTS) {
    await runSingleTest(test.item, true);
  }

  updateOverallByResults();
  appendLog("一键测试完成，最终结果：" + document.getElementById("overallBadge").textContent);
  setButtonsDisabled(false);
}

async function runSingleTest(item, calledByRunAll = false) {
  const test = TESTS.find((entry) => entry.item === item);

  if (!test) {
    return;
  }

  if (!calledByRunAll) {
    setButtonsDisabled(true);
  }

  updateRow({
    item: test.item,
    status: "RUNNING",
    message: "正在测试 " + test.name + "...",
    duration_ms: 0,
  });
  appendLog("开始测试：" + test.name);

  const startTime = Date.now();

  try {
    const data = await requestJson(test.url);
    const result = normalizeResult(data, test, Date.now() - startTime);

    if (result.status === "MANUAL" && result.item === "led") {
      const ok = await showLedConfirm();
      result.status = ok ? "PASS" : "FAIL";
      result.message = ok ? "人工确认 LED 正常闪烁" : "人工确认 LED 未正常闪烁";
    }

    testResults[test.item] = result;
    updateRow(result);
    appendLog(test.name + "：" + result.status + "，" + result.message);

    if (!calledByRunAll) {
      updateOverallByResults();
    }
  } catch (error) {
    const result = {
      item: test.item,
      name: test.name,
      status: "FAIL",
      message: "请求失败：" + error.message,
      duration_ms: Date.now() - startTime,
    };

    testResults[test.item] = result;
    updateRow(result);
    appendLog(test.name + "：FAIL，请求失败：" + error.message);

    if (!calledByRunAll) {
      updateOverallByResults();
    }
  }

  if (!calledByRunAll) {
    setButtonsDisabled(false);
  }
}

async function requestJson(url) {
  const query = new URLSearchParams({ page_time: formatDateTime(new Date()) }).toString();
  const joiner = url.indexOf("?") >= 0 ? "&" : "?";
  const response = await fetch(url + joiner + query, {
    method: "GET",
    cache: "no-store",
  });

  if (!response.ok) {
    throw new Error("HTTP " + response.status);
  }

  const text = await response.text();

  try {
    return JSON.parse(text);
  } catch (error) {
    throw new Error("后端返回不是合法 JSON: " + text.slice(0, 120));
  }
}

function normalizeResult(data, test, duration) {
  const status = String(data.status || "FAIL").toUpperCase();

  return {
    item: data.item || test.item,
    name: data.name || test.name,
    status: STATUS_TEXT[status] ? status : "FAIL",
    message: data.message || "",
    duration_ms: Number(data.duration_ms || duration || 0),
  };
}

function showLedConfirm() {
  const modal = document.getElementById("ledModal");
  modal.style.display = "flex";

  return new Promise((resolve) => {
    ledModalResolver = resolve;
  });
}

function resolveLedConfirm(ok) {
  const modal = document.getElementById("ledModal");
  modal.style.display = "none";

  if (ledModalResolver) {
    const resolve = ledModalResolver;
    ledModalResolver = null;
    resolve(ok);
  }
}

function updateRow(result) {
  const row = document.querySelector('tr[data-item="' + result.item + '"]');

  if (!row) {
    return;
  }

  const status = row.querySelector(".status");
  status.className = "status " + result.status.toLowerCase();
  status.textContent = STATUS_TEXT[result.status] || result.status;

  row.querySelector(".duration").textContent = result.status === "RUNNING" ? "-" : formatDuration(result.duration_ms);
  row.querySelector(".message").textContent = result.message || "-";
}

function updateOverallByResults() {
  const results = Object.values(testResults);

  if (results.length === 0) {
    setOverall("IDLE");
    return;
  }

  if (results.some((result) => result.status === "FAIL")) {
    setOverall("FAIL");
    return;
  }

  if (TESTS.every((test) => testResults[test.item] && testResults[test.item].status === "PASS")) {
    setOverall("PASS");
    return;
  }

  setOverall("IDLE");
}

function setOverall(status) {
  const badge = document.getElementById("overallBadge");
  badge.className = "overall " + status.toLowerCase();
  badge.textContent = STATUS_TEXT[status] || status;
}

function formatDuration(durationMs) {
  const value = Number(durationMs || 0);

  if (value <= 0) {
    return "-";
  }

  if (value < 1000) {
    return value + " ms";
  }

  return (value / 1000).toFixed(2) + " s";
}

function appendLog(message) {
  const logBox = document.getElementById("logBox");

  if (logBox.textContent === "等待开始测试...") {
    logBox.textContent = "";
  }

  logBox.textContent += "[" + formatDateTime(new Date()) + "] " + message + "\n";
  logBox.scrollTop = logBox.scrollHeight;
}

async function startPreview() {
  if (previewStarted) {
    return;
  }

  const preview = document.getElementById("mjpegPreview");
  const hint = document.getElementById("previewHint");

  previewStarted = true;
  preview.src = "/mjpeg?ts=" + Date.now();
  hint.textContent = "正在连接 MJPEG 实时流...";
  appendLog("开始 MJPEG 实时预览");
}

async function endTest() {
  const preview = document.getElementById("mjpegPreview");
  const hint = document.getElementById("previewHint");

  setButtonsDisabled(true);
  preview.removeAttribute("src");
  previewStarted = false;
  hint.textContent = "测试已结束";
  appendLog("结束测试，请求后端退出并保存结果");

  try {
    await requestJson("/api/end_test");
    appendLog("测试已结束，结果文件：" + END_TEST_RESULT_FILE);
  } catch (error) {
    appendLog("结束测试失败：" + error.message);
    setButtonsDisabled(false);
    return;
  }
}

function clearResults() {
  testResults = {};

  for (const test of TESTS) {
    updateRow({
      item: test.item,
      status: "IDLE",
      message: "等待测试",
      duration_ms: 0,
    });
  }

  setOverall("IDLE");
  document.getElementById("logBox").textContent = "等待开始测试...";
}

function setButtonsDisabled(disabled) {
  document.getElementById("runAllBtn").disabled = disabled;
  document.getElementById("clearBtn").disabled = disabled;

  document.querySelectorAll("[data-test]").forEach((button) => {
    button.disabled = disabled;
  });
}
