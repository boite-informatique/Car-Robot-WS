const http = require("http");
const WebSocket = require("ws");
const fs = require("fs");
const path = require("path");
const { detect_objects_on_image } = require("./object_detection");
const sharp = require("sharp");

// Create an HTTP server
const server = http.createServer(async (req, res) => {
  const url = new URL(req.url, `http://${req.headers.host}`);
  const pathname = url.pathname;

  if (
    req.headers["upgrade"] &&
    req.headers["upgrade"].toLowerCase() === "websocket"
  ) {
    if (req.headers["authorization"]) {
      res.writeHead(101, {
        Connection: "Upgrade",
        Upgrade: "websocket",
      });
      return res.end();
    } else {
      res.writeHead(401, {
        "Content-Type": "text/html",
        "WWW-Authenticate": 'Basic realm="some realm"',
      });
      return res.end("<p>must authenticate</p>");
    }
  }

  // Serve static files as in the original code
  const filePath = path.join(
    __dirname,
    "public",
    pathname === "/" ? "index.html" : pathname
  );
  fs.readFile(filePath, (err, data) => {
    if (err) {
      res.writeHead(404, { "Content-Type": "text/html" });
      return res.end("<p>File not found</p>");
    }
    res.writeHead(200, {
      "Content-Type":
        path.extname(filePath) === ".html"
          ? "text/html"
          : "application/octet-stream",
    });
    res.end(data);
  });
});

// Create a WebSocket server
const wss = new WebSocket.Server({ server });

let espSocketClient;
let objDetectionEnabled = false;
let rotationAngle = 0;
let espActions = [
  "forward",
  "backward",
  "left",
  "right",
  "stop",
  "flashon",
  "flashoff",
  "servo",
  "fps",
  "resolution",
];

let serverActions = ["objdetectionOn", "objdetectionOff", "rotate"];

wss.on("connection", (ws, req) => {
  const isCam = req.headers["authorization"] ? true : false;
  if (isCam) {
    espSocketClient = ws;
  }

  ws.data = { isCam };

  ws.on("message", async (message) => {
    if (isCam) {
      const originalBuf = Buffer.from(message);
      const buf =
        rotationAngle == 0
          ? originalBuf
          : await sharp(originalBuf).rotate(rotationAngle).toBuffer(); // rotated

      const boxes = objDetectionEnabled
        ? await detect_objects_on_image(buf)
        : [];
      // Publishing the message to all subscribers
      wss.clients.forEach((client) => {
        if (client.readyState === WebSocket.OPEN && client !== ws) {
          client.send(
            JSON.stringify({
              image: `data:image/jpeg;base64,${buf.toString("base64")}`,
              boxes,
            })
          );
        }
      });
      // fs.writeFileSync("image.jpg", buf);
    } else {
      // not from esp32, from browser client ex
      const action = String(message);
      console.log(`[ACTION] ${action}`);

      if (espSocketClient && espActions.some((a) => action.startsWith(a))) {
        espSocketClient.send(String(action));
      } else if (serverActions.includes(action)) {
        switch (action) {
          case "objdetectionOn":
            objDetectionEnabled = true;
            break;
          case "objdetectionOff":
            objDetectionEnabled = false;
            break;
          case "rotate":
            rotationAngle = (rotationAngle + 90) % 360;
            console.log(rotationAngle);
            break;
          default:
            break;
        }
      }
    }
  });

  ws.on("open", () => {
    console.log(`[new connection] isCam: ${ws.data.isCam}`);
    if (!ws.data.isCam) {
      ws.subscribe("cam-stream");
    }
  });
});

server.listen(3000, () => {
  console.log("Listening on http://localhost:3000");
});
