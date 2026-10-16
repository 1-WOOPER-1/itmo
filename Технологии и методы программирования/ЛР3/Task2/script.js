const loading = document.getElementById("loading-surface");
const networkLabel = document.getElementById("network-label");
const deviceLabel = document.getElementById("device-label");
const networkInfo = document.getElementById("network-info");
const deviceInfo = document.getElementById("device-info");
const cursor = document.getElementById("cursor");

const LOADING_TIME = 2000;

const sleep = (ms) => new Promise((resolve) => setTimeout(resolve, ms));

async function typeText(element, text, speed = 25) {
  element.appendChild(cursor);

  for (let i = 0; i < text.length; i++) {
    cursor.insertAdjacentText("beforebegin", text[i]);
    await sleep(speed);
  }
}

async function getIP() {
  try {
    const res = await fetch("/api/ip");
    const data = await res.json();

    return data;
  } catch (err) {
    throw new Error(arr);
  }
}

function getGPU() {
  const canvas = document.createElement("canvas");
  const gl =
    canvas.getContext("webgl") || canvas.getContext("experimental-webgl");
  if (!gl) return "unknown";

  const debugInfo = gl.getExtension("WEBGL_debug_renderer_info");
  if (!debugInfo) return "unknown";

  return {
    vendor: gl.getParameter(debugInfo.UNMASKED_VENDOR_WEBGL),
    renderer: gl.getParameter(debugInfo.UNMASKED_RENDERER_WEBGL),
  };
}

async function getClientSystemInfo() {
  const gpu = getGPU();

  const info = {
    language: navigator.language,
    languages: navigator.languages,

    screenWidth: screen.width,
    screenHeight: screen.height,
    colorDepth: screen.colorDepth,
    pixelRatio: window.devicePixelRatio,

    cpuCores: navigator.hardwareConcurrency || "unknown",
    deviceMemory: navigator.deviceMemory || "unknown",
    hardwareConcurrency: navigator.hardwareConcurrency,
    gpuVendor: gpu.vendor,
    gpuRenderer: gpu.renderer,

    connectionType: navigator.connection
      ? navigator.connection.effectiveType
      : "unknown",
    downlink: navigator.connection
      ? navigator.connection.downlink + " Mbps"
      : "unknown",
  };

  if (navigator.userAgentData) {
    try {
      const highEntropy = await navigator.userAgentData.getHighEntropyValues([
        "platform",
        "platformVersion",
        "architecture",
        "model",
        "bitness",
      ]);
      info.clientHints = highEntropy;
    } catch (e) {}
  } else {
    info.userAgent = navigator.userAgent;
  }

  return info;
}

async function renderRow(element, key, value, depth = 0) {
  const dataRow = document.createElement("tr");
  const dataKey = document.createElement("td");
  const dataValue = document.createElement("td");

  if (depth > 0) {
    dataKey.style.paddingLeft = `${depth * 20}px`;
    dataKey.style.opacity = "0.85";
  }

  dataRow.appendChild(dataKey);
  dataRow.appendChild(dataValue);
  element.appendChild(dataRow);

  const keyPrefix = depth > 0 ? "↳ " : "";
  await typeText(dataKey, `${keyPrefix}${key}: `, 15);

  if (value !== undefined && value !== null && value !== "") {
    await typeText(dataValue, String(value), 10);
  } else if (value === "") {
    await typeText(dataValue, "—", 10);
  }

  await sleep(40);
}

async function displayData(element, data, depth = 0) {
  if (!data) return;

  if (typeof data === "string") {
    await typeText(element, data);
    return;
  }

  for (const [key, value] of Object.entries(data)) {
    const isObject = typeof value === "object" && value !== null;
    const isArray = Array.isArray(value);

    if (isArray && value.every((item) => typeof item !== "object")) {
      await renderRow(element, key, value.join(", "), depth);
      continue;
    }

    if (isArray && value.length > 0 && typeof value[0] === "object") {
      const formattedBrands = value
        .map((b) =>
          b.brand ? `${b.brand} (v${b.version})` : JSON.stringify(b),
        )
        .join(", ");
      await renderRow(element, key, formattedBrands, depth);
      continue;
    }

    if (isObject) {
      await renderRow(element, key, "", depth);
      await displayData(element, value, depth + 1);
      continue;
    }

    let displayVal = value;
    if (typeof value === "boolean") {
      displayVal = value ? "true" : "false";
    }

    await renderRow(element, key, displayVal, depth);
  }
}

async function init() {
  const ipData = await getIP();
  const systemData = await getClientSystemInfo();

  const res = await fetch("/api/client", {
    method: "POST",
    headers: {
      "Content-Type": "application/json",
    },
    body: JSON.stringify({
      ipData,
      systemData,
    }),
  });

  const req = await res.json();
  console.log(req);
  loading.style.display = "none";

  await displayData(networkLabel, "Network Information");
  await displayData(networkInfo, ipData);
  await displayData(deviceLabel, "Device Information");
  await displayData(deviceInfo, systemData);
}

init();
