const express = require("express");
const fs = require("fs");
var https = require("https");
const app = express();
const bodyParser = require("body-parser");
const cors = require("cors");

const channels = {};

const options = {
  key: fs.readFileSync("key.pem"),
  cert: fs.readFileSync("cert.pem"),
  ca: fs.readFileSync("softlockca.crt"),
  requestCert: true,
  rejectUnauthorized: true,
};

function sendEventsToAll(event, channelId) {
  if (!channels[channelId]) {
    channels[channelId] = [];
  }

  channels[channelId].forEach((c) =>
    c.res.write(`data: ${JSON.stringify(event)}\n\n`)
  );
}

app.use(cors());
app.use(bodyParser.json());
app.use(bodyParser.urlencoded({ extended: false }));

app.post("/:channelId/send", (req, res, next) => {
  const { channelId } = req.params;

  prepareResponse(req.body, res).then((response) => {
    sendEventsToAll(response, channelId);
  });

  return res.send("ok");
});

//Subscribe to the server
app.get("/:channelId/listen", function (req, res) {
  console.log("hello");
  res.writeHead(200, {
    "Content-Type": "text/event-stream",
    Connection: "keep-alive",
    "Cache-Control": "no-cache",
  });

  const { channelId } = req.params;
  const clientId = Date.now();

  if (!channels[channelId]) {
    channels[channelId] = [];
  }

  channels[channelId].push({
    id: clientId,
    res,
  });

  const data = `data: ${JSON.stringify([
    {
      username: "Bot",
      message: "Welcome! Happy to see you ;)",
      time: Date.now(),
    },
  ])}\n\n`;

  res.write(data);

  req.on("close", () => {
    console.log(`${clientId} Connection closed`);
    channels[channelId] = channels[channelId].filter((c) => c.id !== clientId);
  });
});

https.createServer(options, app).listen(3000, function () {
    console.log("SSE Tchat listening on port 3000!");
  });



//Helper functions-----------------------------
prepareResponse = (requestObject, res) => {
  return new Promise((resolve, rej) => {
    let op = requestObject["op"];
    let num = requestObject["num"];
    let num1 = requestObject["num1"];
    console.log("requestObject", requestObject);
    let response = {};

    switch (op) {
      case "diff":
        subtract(num, num1).then((ans) => {
          resolve(
            (response = {
              op: "subtract",
              result: ans,
            })
          );
        });

        break;
      case "add":
        add(num, num1).then((ans) => {
          resolve(
            (response = {
              op: "add",
              result: ans,
            })
          );
        });
        break;
      default:
        resolve({});
        break;
    }
  });
};

//add
add = (a, b) => {
  return new Promise((resolve, reject) => {
    let t = Math.floor(Math.random() * 20) * 1000;
    console.log("time: ", t);
    setTimeout(() => {
      let res = parseInt(a) + parseInt(b);
      resolve(res);
    }, t);
  });
};

subtract = (a, b) => {
  return new Promise((resolve, reject) => {
    let t = Math.floor(Math.random() * 20) * 1000;
    console.log("time: ", t);
    setTimeout(() => {
      let res = parseInt(a) - parseInt(b);
      resolve(res);
    }, t);
  });
};
