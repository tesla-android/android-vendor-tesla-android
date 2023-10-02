'use strict';
const MANIFEST = 'flutter-app-manifest';
const TEMP = 'flutter-temp-cache';
const CACHE_NAME = 'flutter-app-cache';

const RESOURCES = {"h264.js": "36ea412ec86751be2a5d7a14aafc5e07",
"version.json": "fee417dfa0cfaa93b7aba6a4335cb947",
"favicon.ico": "1a96a32111338f8aa97efc700d8a7612",
"index.html": "0436bbe90c00c70826aaa0c959689651",
"/": "0436bbe90c00c70826aaa0c959689651",
"imgTag.js": "a5870d73cba2f73c28b9b5f0820d93b9",
"workerWebGLWebCodecs.js": "7f3bff3e2bb1cd5190fe257a6ab19def",
"main.dart.js": "309932b00fc9df6db4b343d19345cf12",
"reconnecting-websocket.js": "4ef9dd81c9e8157a1702623078c7ece1",
"flutter.js": "6fef97aeca90b426343ba6c5c9dc5d4a",
"estimator.js": "2e8d77774febade25070f24689662912",
"icons/favicon-16x16.png": "dc6aaa006f55ef3f9f68fd1594072920",
"icons/apple-icon.png": "8a2eb7cf60c1ce4d9655761391a69185",
"icons/apple-icon-144x144.png": "83fed74b15d3c7b7b13e9c40baf1d26e",
"icons/android-icon-192x192.png": "22930c0932e6b591c2a493ec78fc95ed",
"icons/apple-icon-precomposed.png": "8a2eb7cf60c1ce4d9655761391a69185",
"icons/apple-icon-114x114.png": "3a6d69dd6a52aa829cf9aa55687b8ca1",
"icons/ms-icon-310x310.png": "b853b01d1fb33a844a2f8c58dded416c",
"icons/ms-icon-144x144.png": "83fed74b15d3c7b7b13e9c40baf1d26e",
"icons/apple-icon-57x57.png": "f4a2df4af97bab4981f98d8611e47789",
"icons/apple-icon-152x152.png": "9fcff3a3ebef4c36c07ad79dae66d1a6",
"icons/ms-icon-150x150.png": "4e21fef049d260e2e690c451d2eaa025",
"icons/android-icon-72x72.png": "b29453bd170afe9c452a0444abd3813f",
"icons/android-icon-96x96.png": "23676968255bc351db91ed70c1d971cd",
"icons/android-icon-36x36.png": "3fac0427277c54686e9f4a4beda2be2d",
"icons/apple-icon-180x180.png": "7f201c1162fa0c0cdc29e0ca3643c750",
"icons/favicon-96x96.png": "2c0213b57ef83c3698f6f25399ef135d",
"icons/manifest.json": "b58fcfa7628c9205cb11a1b2c3e8f99a",
"icons/android-icon-48x48.png": "71ec42e6b6ce0a62458bb01a4ec2dd33",
"icons/apple-icon-76x76.png": "854220810d20a461d5ef46ca5ac4a5e8",
"icons/apple-icon-60x60.png": "7af97e474b3fc4cf73b529b3e7879b66",
"icons/android-icon-144x144.png": "e07c890a192fa53daec0d96a42be0567",
"icons/apple-icon-72x72.png": "80f8d85a567fa6f7bdf6a106199521f5",
"icons/apple-icon-120x120.png": "8712d55f56578850bd6fb7387fc70038",
"icons/favicon-32x32.png": "fefee8d06ae6c37d5a403872fb630186",
"icons/ms-icon-70x70.png": "d3cfc04b96a3a9e32a55baabf2f9de3f",
"pcmplayer-processor.js": "91b956d9528159576954ac70c754cd21",
"android.html": "85e121319bd8d9150015a2fc38c4ecbf",
"audioplayback.js": "d87dbba06b9040a70e06c7cbfd9b4c28",
"h264_worker.js": "ce98425222ba02b91e3df2c2dc7a622a",
"online/status.html": "d3f6832b8fb04c5b14bd759481f1c89d",
"online/connectivity_check.txt": "7f46165474d11ee5836777d85df2cdab",
"assets/images/png/tesla-android-logo.png": "9ce39021f18753f2c21a4ae08ae198ce",
"assets/AssetManifest.json": "83d94fdc4d63e7e0e0439f8c18b8fafd",
"assets/NOTICES": "2016ae90c744817d1051e235f6f765ec",
"assets/FontManifest.json": "f589285d193a17a5dcaef1fc3e0e834f",
"assets/shaders/ink_sparkle.frag": "f8b80e740d33eb157090be4e995febdf",
"assets/AssetManifest.bin": "61786063d67195fab32d9d389ddd21f3",
"assets/fonts/Roboto-Regular.ttf": "8a36205bd9b83e03af0591a004bc97f4",
"assets/fonts/MaterialIcons-Regular.otf": "fb022c541a3ee392517c32389d182d12",
"webGLWebCodecs.js": "8bf99f10a377a1c4885f77f206f50d33",
"browserconfig.xml": "04007f423f714a53a3476f43f34c9cdf",
"canvaskit/skwasm.js": "95f16c6690f955a45b2317496983dbe9",
"canvaskit/skwasm.wasm": "d1fde2560be92c0b07ad9cf9acb10d05",
"canvaskit/chromium/canvaskit.js": "ffb2bb6484d5689d91f393b60664d530",
"canvaskit/chromium/canvaskit.wasm": "393ec8fb05d94036734f8104fa550a67",
"canvaskit/canvaskit.js": "5caccb235fad20e9b72ea6da5a0094e6",
"canvaskit/canvaskit.wasm": "d9f69e0f428f695dc3d66b3a83a4aa8e",
"canvaskit/skwasm.worker.js": "51253d3321b11ddb8d73fa8aa87d3b15"};
// The application shell files that are downloaded before a service worker can
// start.
const CORE = ["main.dart.js",
"index.html",
"assets/AssetManifest.json",
"assets/FontManifest.json"];

// During install, the TEMP cache is populated with the application shell files.
self.addEventListener("install", (event) => {
  self.skipWaiting();
  return event.waitUntil(
    caches.open(TEMP).then((cache) => {
      return cache.addAll(
        CORE.map((value) => new Request(value, {'cache': 'reload'})));
    })
  );
});
// During activate, the cache is populated with the temp files downloaded in
// install. If this service worker is upgrading from one with a saved
// MANIFEST, then use this to retain unchanged resource files.
self.addEventListener("activate", function(event) {
  return event.waitUntil(async function() {
    try {
      var contentCache = await caches.open(CACHE_NAME);
      var tempCache = await caches.open(TEMP);
      var manifestCache = await caches.open(MANIFEST);
      var manifest = await manifestCache.match('manifest');
      // When there is no prior manifest, clear the entire cache.
      if (!manifest) {
        await caches.delete(CACHE_NAME);
        contentCache = await caches.open(CACHE_NAME);
        for (var request of await tempCache.keys()) {
          var response = await tempCache.match(request);
          await contentCache.put(request, response);
        }
        await caches.delete(TEMP);
        // Save the manifest to make future upgrades efficient.
        await manifestCache.put('manifest', new Response(JSON.stringify(RESOURCES)));
        // Claim client to enable caching on first launch
        self.clients.claim();
        return;
      }
      var oldManifest = await manifest.json();
      var origin = self.location.origin;
      for (var request of await contentCache.keys()) {
        var key = request.url.substring(origin.length + 1);
        if (key == "") {
          key = "/";
        }
        // If a resource from the old manifest is not in the new cache, or if
        // the MD5 sum has changed, delete it. Otherwise the resource is left
        // in the cache and can be reused by the new service worker.
        if (!RESOURCES[key] || RESOURCES[key] != oldManifest[key]) {
          await contentCache.delete(request);
        }
      }
      // Populate the cache with the app shell TEMP files, potentially overwriting
      // cache files preserved above.
      for (var request of await tempCache.keys()) {
        var response = await tempCache.match(request);
        await contentCache.put(request, response);
      }
      await caches.delete(TEMP);
      // Save the manifest to make future upgrades efficient.
      await manifestCache.put('manifest', new Response(JSON.stringify(RESOURCES)));
      // Claim client to enable caching on first launch
      self.clients.claim();
      return;
    } catch (err) {
      // On an unhandled exception the state of the cache cannot be guaranteed.
      console.error('Failed to upgrade service worker: ' + err);
      await caches.delete(CACHE_NAME);
      await caches.delete(TEMP);
      await caches.delete(MANIFEST);
    }
  }());
});
// The fetch handler redirects requests for RESOURCE files to the service
// worker cache.
self.addEventListener("fetch", (event) => {
  if (event.request.method !== 'GET') {
    return;
  }
  var origin = self.location.origin;
  var key = event.request.url.substring(origin.length + 1);
  // Redirect URLs to the index.html
  if (key.indexOf('?v=') != -1) {
    key = key.split('?v=')[0];
  }
  if (event.request.url == origin || event.request.url.startsWith(origin + '/#') || key == '') {
    key = '/';
  }
  // If the URL is not the RESOURCE list then return to signal that the
  // browser should take over.
  if (!RESOURCES[key]) {
    return;
  }
  // If the URL is the index.html, perform an online-first request.
  if (key == '/') {
    return onlineFirst(event);
  }
  event.respondWith(caches.open(CACHE_NAME)
    .then((cache) =>  {
      return cache.match(event.request).then((response) => {
        // Either respond with the cached resource, or perform a fetch and
        // lazily populate the cache only if the resource was successfully fetched.
        return response || fetch(event.request).then((response) => {
          if (response && Boolean(response.ok)) {
            cache.put(event.request, response.clone());
          }
          return response;
        });
      })
    })
  );
});
self.addEventListener('message', (event) => {
  // SkipWaiting can be used to immediately activate a waiting service worker.
  // This will also require a page refresh triggered by the main worker.
  if (event.data === 'skipWaiting') {
    self.skipWaiting();
    return;
  }
  if (event.data === 'downloadOffline') {
    downloadOffline();
    return;
  }
});
// Download offline will check the RESOURCES for all files not in the cache
// and populate them.
async function downloadOffline() {
  var resources = [];
  var contentCache = await caches.open(CACHE_NAME);
  var currentContent = {};
  for (var request of await contentCache.keys()) {
    var key = request.url.substring(origin.length + 1);
    if (key == "") {
      key = "/";
    }
    currentContent[key] = true;
  }
  for (var resourceKey of Object.keys(RESOURCES)) {
    if (!currentContent[resourceKey]) {
      resources.push(resourceKey);
    }
  }
  return contentCache.addAll(resources);
}
// Attempt to download the resource online before falling back to
// the offline cache.
function onlineFirst(event) {
  return event.respondWith(
    fetch(event.request).then((response) => {
      return caches.open(CACHE_NAME).then((cache) => {
        cache.put(event.request, response.clone());
        return response;
      });
    }).catch((error) => {
      return caches.open(CACHE_NAME).then((cache) => {
        return cache.match(event.request).then((response) => {
          if (response != null) {
            return response;
          }
          throw error;
        });
      });
    })
  );
}
