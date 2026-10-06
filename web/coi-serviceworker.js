/*
 * GitHub Pages cross-origin isolation helper.
 *
 * GitHub Pages cannot set arbitrary COOP/COEP response headers. This service
 * worker upgrades same-origin navigations and subresources to the headers
 * required by WebAssembly pthreads/SharedArrayBuffer.
 */
(() => {
  const VERSION = 'bryairs-console-mc-coi-1';
  const scope = self.registration ? self.registration.scope : './';

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
