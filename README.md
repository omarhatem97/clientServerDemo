# Qt-Server-Sent-Events-Demo

A Qt5 Https Server Sent Events (SSE) demo.

## Build instructions

- `cmake CMakeLists.txt`
- `make`
- `qt-sse-demo.exe <URL> <add/diff> <num> <num1>`

## Demo

**Setup SSE server**

This repo includes a simple SSE server demo in the `server` directory.
To run it, you need NodeJS and Yarn installed:

- `cd server`
- `yarn install`
- `yarn start`

The demo server runs on port `3000` and publishes events on `/{channelId}/listen`.
and the following requests get sent to `/{channelId}/send`.

**Demo time!**

first, make sure to update the configuration.ini file with the correct certificates and key paths then copy it to the Qt client application executable folder.

Start the Qt SSE demo with our SSE demo server as URL:

examples:

`qt-sse-demo.exe http://localhost:3000/ diff 5 3' `qt-sse-demo.exe http://localhost:3000/ add 2 4'

the server wait for a random time then send response to subscriber at http://localhost:3000/{channelId}/listen

based on Qt Server Sent Events demo, written by Dylan Van Assche.
