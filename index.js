const http = require("http");
const WebSocket = require("ws");
const fs = require("fs");
const path = require("path");
const { detect_objects_on_image } = require("./object_detection");

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

wss.on("connection", (ws, req) => {
  const isCam = req.headers["authorization"] ? true : false;
  if (isCam) {
    espSocketClient = ws;
  }

  ws.data = { isCam };

  ws.on("message", async (message) => {
    if (isCam) {
      const buf = Buffer.from(message);
      const boxes = await detect_objects_on_image(buf);

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
    } else {
      // not from esp32, from browser client ex
      console.log(`[ACTION] ${message}`);
      if (espSocketClient) {
        ws.send(message);
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
