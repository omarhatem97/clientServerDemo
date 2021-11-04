/*
 * SSE server demo
 * (c) Dylan Van Assche (2020)
 * Released under the GPLv3 license
 * Based on: https://alligator.io/nodejs/server-sent-events-build-realtime-app/
 */
const express = require("express");
const cors = require("cors");
const app = express();
const PORT = 3000;
const INTERVAL = 10000;

const https = require("https");
const fs = require("fs");
const url = require("url");

app.use(express.json());

// SSE headers
const headers = {
  "Content-Type": "text/event-stream",
  Connection: "keep-alive",
  "Cache-Control": "no-cache",
};

// Client management
let clientIdCounter = 0;
let clients = [];

var requests = [];

add = (a, b) => {
  return a === undefined || b === undefined ? 0 : a + b;
};
diff = (a, b) => {
  return a === undefined || b === undefined ? 0 : a - b;
};

// Send events to all clients using SSE
function sendEvents() {
  console.info("Sending events to all clients");
  //var funName, addres, diffres
  var op = "";
  var num = 0;
  var num1 = 0;
  var res;
  if (requests.size === 0) return;
  else {
    var currRequest = requests.pop();
    //console.log(currRequest);
    if (currRequest !== undefined) {
      op = currRequest["op"];
      num = currRequest["num"];
      num1 = currRequest["num1"];
    } else return;

    if (op === "add") {
      res = add(num, num1);
      op = "ADD";
    } else {
      res = diff(num, num1);
      op = "SUBTRACT";
    }
  }
  let data = {
    timestamp: `${new Date()}`,
    message: "Softlock Broadcasting!",
    Result: res,
    FunctionName: op,
  };
  clients.forEach((c) => c.res.write(`data: ${JSON.stringify(data)}\n\n`));
}

const options = {
  key: fs.readFileSync("key.pem"),
  cert: fs.readFileSync("cert.pem"),
  ca: fs.readFileSync("softlockca.crt"),
  requestCert: true,
  rejectUnauthorized: true,
};

https
  .createServer(options, function (req, res) {
    queryObject = url.parse(req.url, true).query;

    //handling post requests
    if (req.method == "POST") {
      var body = "";
      req.on("data", function (data) {
        body = JSON.parse(data);
        var op = body["op"];
        var num = parseInt(body["num"]);
        var num1 = parseInt(body["num1"]);

        requests.push({
          op: op,
          num: num,
          num1: num1,
        });
      });
      req.on("end", function () {
        console.log("Body: " + body);
        res.writeHead(200, { "Content-Type": "text/html" });
        res.end("post received");
      });
    }
    res.writeHead(200, headers);
    let data = {
      timestamp: `${new Date()}`,
      message: "Hello World!",
    };
    res.write(`data: ${JSON.stringify(data)}\n\n`);

    let clientId = clientIdCounter;
    const client = {
      id: clientId,
      res,
    };
    clients.push(client);
    clientIdCounter++;

    // Remove client from list if connection is closed
    req.on("close", () => {
      console.log(`${clientId} Connection closed`);
      clients = clients.filter((c) => c.id !== clientId);
    });

    console.log("Listener Count: " + req.listenerCount());
    setInterval(sendEvents, INTERVAL);
  })
  .listen(PORT, () => console.log(`SSE server demo listening on port ${PORT}`));
