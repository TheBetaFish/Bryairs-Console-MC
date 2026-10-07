(() => {
  const canvas = document.getElementById('canvas');
  const fullscreen = document.getElementById('fullscreen');
  const settings = document.getElementById('settings');
  const panel = document.getElementById('web-panel');
  const scale = document.getElementById('render-scale');
  const scaleValue = document.getElementById('scale-value');
  const diagnostics = document.getElementById('web-diagnostics');
  let frames = 0;
  let fps = 0;
  let fpsTime = performance.now();

  function resizeCanvas() {
    const factor = Number(scale.value) / 100;
    const dpr = Math.min(window.devicePixelRatio || 1, 2);
    const width = Math.max(1, Math.floor(window.innerWidth * dpr * factor));
    const height = Math.max(1, Math.floor(window.innerHeight * dpr * factor));
    if (window.Module && typeof Module.setCanvasSize === 'function') {
      Module.setCanvasSize(width, height, true);
    } else {
      canvas.width = width;
      canvas.height = height;
    }
  }

  function setScale(value) {
    scale.value = String(value);
    scaleValue.textContent = value + '%';
    resizeCanvas();
    try { localStorage.setItem('bryairs-web-render-scale', String(value)); } catch (_) {}
  }

  fullscreen.addEventListener('click', async () => {
    try {
      if (!document.fullscreenElement) await canvas.requestFullscreen();
      else await document.exitFullscreen();
    } catch (error) {
      console.warn('Fullscreen unavailable:', error);
    }
  });

  settings.addEventListener('click', () => {
    panel.style.display = panel.style.display === 'block' ? 'none' : 'block';
  });

  scale.addEventListener('input', () => setScale(scale.value));
  window.addEventListener('resize', resizeCanvas);
  document.addEventListener('fullscreenchange', resizeCanvas);

  try {
    const saved = Number(localStorage.getItem('bryairs-web-render-scale'));
    if (saved >= 50 && saved <= 100) setScale(saved);
  } catch (_) {}

  function updateDiagnostics() {
    if (!diagnostics) return;
    const parts = [
      'FPS: ' + fps,
      'Viewport: ' + window.innerWidth + 'x' + window.innerHeight,
      'Canvas: ' + canvas.width + 'x' + canvas.height,
      'Gamepads: ' + (navigator.getGamepads ? Array.from(navigator.getGamepads()).filter(Boolean).length : 0)
    ];
    const gl = canvas.getContext('webgl2');
    if (gl) {
      const renderer = gl.getParameter(gl.RENDERER);
      if (renderer) parts.push('WebGL: ' + renderer);
    }
    const memory = performance.memory;
    if (memory) {
      parts.push('JS heap: ' + Math.round(memory.usedJSHeapSize / 1048576) +
        ' / ' + Math.round(memory.jsHeapSizeLimit / 1048576) + ' MB');
    }
    diagnostics.textContent = parts.join('\n');
  }

  function frameCounter(now) {
    frames++;
    if (now - fpsTime >= 1000) {
      fps = frames;
      frames = 0;
      fpsTime = now;
    }
    requestAnimationFrame(frameCounter);
  }

  function reportError(message) {
    const status = document.getElementById('status');
    if (status) status.textContent = 'WebAssembly error: ' + message;
    console.error(message);
  }

  window.addEventListener('error', event => {
    if (event.error || event.message) reportError(event.error?.message || event.message);
  });
  window.addEventListener('unhandledrejection', event => {
    reportError(event.reason?.message || String(event.reason));
  });
  window.addEventListener('gamepadconnected', updateDiagnostics);
  window.addEventListener('gamepaddisconnected', updateDiagnostics);

  requestAnimationFrame(frameCounter);
  setInterval(updateDiagnostics, 2000);
  resizeCanvas();
  updateDiagnostics();
})();
