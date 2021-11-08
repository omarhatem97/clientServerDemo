//
/* Client Code 
let sse = new EventSource("http://localhost:8080/stream");
sse.onmessage = console.log
*/

const app = require("express")();
const fs = require("fs");
var https = require("https");
const port = process.env.PORT || 8888;
const serverName = process.env.SERVER_NAME || "sample";

//global variables
let requestObject = null;

const options = {
  key: fs.readFileSync("key.pem"),
  cert: fs.readFileSync("cert.pem"),
  ca: fs.readFileSync("softlockca.crt"),
  requestCert: true,
  rejectUnauthorized: true,
};

app.get("/", (req, res) => res.send("hello!"));

app.get("/stream", (req, res) => {
  res.setHeader("Content-Type", "text/event-stream");
  send(res);
});

app.post("/stream", (req, res) => {
  res.setHeader("Content-Type", "text/event-stream");

  req.on("data", (data) => {
    requestObject = JSON.parse(data);
    send(res);
    //console.log(requestObject);
  });
});


let i = 0;

function send(res) {
  res.write("data: " + `hello from ${serverName} ---- [${i++}]\n\n`);

  while (requestObject === null)
    continue;

  console.log(requestObject);
  //logic
  let op = requestObject["op"];
  let num = requestObject["num"];
  let num1 = requestObject["num1"];

  switch (op) {
    case "diff":
      res.write(
        "data: " + `Diff operation occured and result is ${num - num1} ----\n\n`
      );
      break;
    case "add" :
      res.write(
        "data: " + `Diff operation occured and result is ${num - num1} ----\n\n`
      );
      break;
    default:
      break;
  }
  //endlogic
  //console.log("data is :", req.body);
  setTimeout(() => send(res), 1000);
}

https.createServer(options, app).listen(port);
//app.listen(port);
console.log(`Listening on ${port}`);

// /*

// const express = require("express");
// const cors = require("cors");
// const app = express();
// const PORT = 3000;
// const INTERVAL = 10000;

// const https = require("https");
// const fs = require("fs");
// const url = require("url");

// app.use(express.json());

// // SSE headers
// const headers = {
//   "Content-Type": "text/event-stream",
//   Connection: "keep-alive",
//   "Cache-Control": "no-cache",
// };

// // Client management
// let clientIdCounter = 0;
// let clients = [];

// var requests = [];

// add = (a, b) => {
//   return a === undefined || b === undefined ? 0 : a + b;
// };
// diff = (a, b) => {
//   return a === undefined || b === undefined ? 0 : a - b;
// };

// // Send events to all clients using SSE
// function sendEvents() {
//   console.info("Sending events to all clients");
//   //var funName, addres, diffres
//   var op = "";
//   var num = 0;
//   var num1 = 0;
//   var res;
//   if (requests.size === 0) return;
//   else {
//     var currRequest = requests.pop();
//     //console.log(currRequest);
//     if (currRequest !== undefined) {
//       op = currRequest["op"];
//       num = currRequest["num"];
//       num1 = currRequest["num1"];
//     } else return;

//     if (op === "add") {
//       res = add(num, num1);
//       op = "ADD";
//     } else {
//       res = diff(num, num1);
//       op = "SUBTRACT";
//     }
//   }
//   let data = {
//     timestamp: `${new Date()}`,
//     message: "Softlock Broadcasting!",
//     Result: res,
//     FunctionName: op,
//   };
//   clients.forEach((c) => c.res.write(`data: ${JSON.stringify(data)}\n\n`));
// }

// const options = {
//   key: fs.readFileSync("key.pem"),
//   cert: fs.readFileSync("cert.pem"),
//   ca: fs.readFileSync("softlockca.crt"),
//   requestCert: true,
//   rejectUnauthorized: true,
// };

// app.post("/stream", (req, res) => {
//   res.setHeader("Content-Type", "text/event-stream");
//   send(res);
// });

// function send(res) {
//   res.write("data: " + `hello from SERVER ---- [${i++}]\n\n`);

//   setTimeout(() => send(res), 1000);
// }

// https
//   .createServer(options, function (req, res) {
//     queryObject = url.parse(req.url, true).query;

//     //handling post requests
//     if (req.method == "POST") {
//       var body = "";
//       req.on("data", function (data) {
//         body = JSON.parse(data);
//         var op = body["op"];
//         var num = parseInt(body["num"]);
//         var num1 = parseInt(body["num1"]);

//         requests.push({
//           op: op,
//           num: num,
//           num1: num1,
//         });
//       });
//       req.on("end", function () {
//         console.log("Body: " + body);
//         res.writeHead(200, { "Content-Type": "text/html" });
//         res.end("post received");
//       });
//     }
//     res.writeHead(200, headers);
//     let data = {
//       timestamp: `${new Date()}`,
//       message: "Hello World!",
//     };
//     res.write(`data: ${JSON.stringify(data)}\n\n`);

//     let clientId = clientIdCounter;
//     const client = {
//       id: clientId,
//       res,
//     };
//     clients.push(client);
//     clientIdCounter++;
//     console.log("clinets counter:", clientIdCounter);
//     // Remove client from list if connection is closed
//     req.on("close", () => {
//       console.log(`${clientId} Connection closed`);
//       clients = clients.filter((c) => c.id !== clientId);
//       clientIdCounter--;
//     });

//     console.log("Listener Count: " + req.listenerCount());
//     setInterval(sendEvents, INTERVAL);
//   })
//   .listen(PORT, () => console.log(`SSE server demo listening on port ${PORT}`));
