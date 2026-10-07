/*
 * GitHub Pages cross-origin isolation helper.
 *
 * GitHub Pages cannot set arbitrary COOP/COEP response headers. This service
 * worker upgrades same-origin responses to the headers required by
 * WebAssembly pthreads/SharedArrayBuffer.
 *
 * Important: do not copy Content-Encoding/Content-Length from the original
 * response. fetch() may transparently decode compressed responses, and keeping
 * those headers on a new Response can make large Emscripten .data downloads
 * look truncated/corrupt to the application.
 */
(() => {
  const VERSION = 'bryairs-console-mc-coi-2';

  self.addEventListener('install', event => {
    self.skipWaiting();
  });

  self.addEventListener('activate', event => {
    event.waitUntil(self.clients.claim());
  });

  self.addEventListener('fetch', event => {
    const request = event.request;
    if (request.cache === 'only-if-cached' && request.mode !== 'same-origin') return;

    event.respondWith((async () => {
      const response = await fetch(request);

      if (new URL(request.url).origin !== self.location.origin) {
        return response;
      }

      const headers = new Headers(response.headers);
      headers.delete('Content-Encoding');
      headers.delete('Content-Length');
      headers.delete('Content-Range');
      headers.set('Cross-Origin-Opener-Policy', 'same-origin');
      headers.set('Cross-Origin-Embedder-Policy', 'require-corp');
      headers.set('Cross-Origin-Resource-Policy', 'same-origin');

      return new Response(response.body, {
        status: response.status,
        statusText: response.statusText,
        headers
      });
    })());
  });
})();
