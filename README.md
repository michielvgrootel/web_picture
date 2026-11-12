# Web picture

> Pictures come only from the hosts listed under `permissions.network` in `tessera-plugin.yaml`. Edit that list before
> you add the plugin. Addresses on your home network cannot be used (see "Good to know").

A tile that shows the picture at an http(s) address: a webcam snapshot, a weather map, a chart a site renders as an
image. It can fetch the picture again every minute, 5 minutes, 15 minutes or hour, so a picture that changes at the
same address stays current.

## Set up

1. Open `tessera-plugin.yaml` and put the host of every picture you want to show under `permissions.network`, for
   example `network: [images.example.org, cam.example.net]`. At most 8 hosts, public names only.
2. Copy this folder to `tessera-plugins/web_picture/` beside your ESPHome folder (`/config/esphome/tessera-plugins/`).
3. In Tessera, open Plugins, add **Web picture** from the folder to a screen. The screen builds once.
4. In Layout, place the **Web picture** tile from the library's Plugins group.
5. In the inspector, fill in the address. For an address over 64 characters, put the rest in **Address, continued**.
6. Choose **Refresh** for a picture that changes, and **Fit**: cover fills the tile, contain shows all of it.

## Good to know

- Tessera fetches the picture in the app, not on the screen, and only from the hosts in the manifest. It refuses IP
  addresses, redirects and anything that resolves to a private address, and a picture may be at most 2 MB.
- So a camera or a server on your own network cannot be shown this way. For those, add the address in Home Assistant
  as a Generic Camera (it takes any still image URL, local ones too) and put that camera on the screen with Tessera's
  own camera tile.
- With refresh on, the tile adds `_t=<number>` to the address's query, a number that changes on each refresh. Almost
  every server ignores it; if yours refuses it, set Refresh to Never.
- The CYD, the Waveshare 3.5-inch and the Hosyond 4-inch have no memory for pictures: the tile says so there.

## Privacy

The Tessera app in Home Assistant fetches the address you fill in. The host sees your home's IP address and the
request, as it would from a browser. Nothing else is sent.
