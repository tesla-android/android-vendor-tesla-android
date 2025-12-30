'use strict';
const MANIFEST = 'flutter-app-manifest';
const TEMP = 'flutter-temp-cache';
const CACHE_NAME = 'flutter-app-cache';

const RESOURCES = {"flutter_bootstrap.js": "d90bd8b16ab38d5797d674116a9b3c8b",
"version.json": "1c1216197b2c71ed3d5b1d374904fd5c",
"favicon.ico": "1a96a32111338f8aa97efc700d8a7612",
"index.html": "2a3ce84693bbc0358163ea001c3154c3",
"/": "2a3ce84693bbc0358163ea001c3154c3",
"main.dart.js": "dd900b9a9353639371d0ba6ac494bcda",
"reconnecting-websocket.js": "4ef9dd81c9e8157a1702623078c7ece1",
"Decoder.js": "7754075ec40cf97622ccd750cb9031de",
"flutter.js": "4b2350e14c6650ba82871f60906437ea",
"audio-ws-worker.js": "75e004b85ee8b049fe7965c07dc9cf44",
"avc.wasm": "498d2dd44c9fd59639e36bbaabbd6361",
"estimator.js": "2e8d77774febade25070f24689662912",
"h264WebCodecsWorker.js": "ce98425222ba02b91e3df2c2dc7a622a",
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
"h264Brodway.js": "7e2c973f89b4c773642f8bc5958f4758",
"YUVCanvas.js": "e5d2136cde7cf30cbd2207b08b5433c4",
"h264BrodwayWorker.js": "45fdbf4afe8ed9aa36a77345d98441f8",
"android.html": "36a317f725ed3dc45701388f8e72e486",
"h264WebCodecs.js": "27b35df3584646e7c59ba331c647b514",
"audioplayback.js": "e4d56b10ff558dd326fd5dc41738cd14",
"online/status.html": "d3f6832b8fb04c5b14bd759481f1c89d",
"online/connectivity_check.txt": "7f46165474d11ee5836777d85df2cdab",
"assets/images/png/tesla-android-logo.png": "9ce39021f18753f2c21a4ae08ae198ce",
"assets/AssetManifest.json": "83d94fdc4d63e7e0e0439f8c18b8fafd",
"assets/NOTICES": "7df563557ded8386b53dec8f4cf095bc",
"assets/FontManifest.json": "f589285d193a17a5dcaef1fc3e0e834f",
"assets/AssetManifest.bin.json": "c5b36c8963983e4f8f135c5d8d6463d6",
"assets/shaders/ink_sparkle.frag": "ecc85a2e95f5e9f53123dcaf8cb9b6ce",
"assets/AssetManifest.bin": "61786063d67195fab32d9d389ddd21f3",
"assets/fonts/Roboto-Regular.ttf": "8a36205bd9b83e03af0591a004bc97f4",
"assets/fonts/MaterialIcons-Regular.otf": "69dcba94378c0d81e0bde6082aa9d5d5",
"browserconfig.xml": "04007f423f714a53a3476f43f34c9cdf",
"mjpeg.js": "a5870d73cba2f73c28b9b5f0820d93b9",
"canvaskit/skwasm.js": "ac0f73826b925320a1e9b0d3fd7da61c",
"canvaskit/skwasm.js.symbols": "96263e00e3c9bd9cd878ead867c04f3c",
"canvaskit/canvaskit.js.symbols": "efc2cd87d1ff6c586b7d4c7083063a40",
"canvaskit/skwasm.wasm": "828c26a0b1cc8eb1adacbdd0c5e8bcfa",
"canvaskit/chromium/canvaskit.js.symbols": "e115ddcfad5f5b98a90e389433606502",
"canvaskit/chromium/canvaskit.js": "b7ba6d908089f706772b2007c37e6da4",
"canvaskit/chromium/canvaskit.wasm": "ea5ab288728f7200f398f60089048b48",
"canvaskit/canvaskit.js": "26eef3024dbc64886b7f48e1b6fb05cf",
"canvaskit/canvaskit.wasm": "e7602c687313cfac5f495c5eac2fb324",
"canvaskit/skwasm.worker.js": "89990e8c92bcb123999aa81f7e203b1c"};
// The application shell files that are downloaded before a service worker can
// start.
const CORE = ["main.dart.js",
"index.html",
"flutter_bootstrap.js",
"assets/AssetManifest.bin.json",
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
