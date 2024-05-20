const server = Bun.serve<{ isCam: boolean }>({
  port: 3000,
  async fetch(req, server) {
    const success = server.upgrade(req, {
      data: { isCam: req.headers.has("authorization") },
    });
    if (success) {
      // Bun automatically returns a 101 Switching Protocols
      // if the upgrade succeeds
      return undefined;
    }

    // return new Response("<p>must authenticate</p>", {
    //   status: 401,
    //   headers: {
    //     "Content-Type": "text/html",
    //     "WWW-Authenticate": `Basic realm="some realm"`,
    //   },
    // });
    const pathname = new URL(req.url).pathname;
    // if (pathname.startsWith("/face-api.min.js")) {
    //   return new Response(await Bun.file("./public/face-api.min.js"));
    // }

    // if (pathname.startsWith("/face-api.js")) {
    //   return new Response(await Bun.file("./public/face-api.js"));
    // }

    // if (pathname.startsWith("/tiny_face_detector_model-shard1")) {
    //   return new Response(
    //     await Bun.file("./public/tiny_face_detector_model-shard1")
    //   );
    // }
    // if (pathname.startsWith("/face.jpg")) {
    //   return new Response(await Bun.file("./public/face.jpg"));
    // }
    // if (
    //   pathname.startsWith("/tiny_face_detector_model-weights_manifest.json")
    // ) {
    //   return new Response(
    //     await Bun.file(
    //       "./public/tiny_face_detector_model-weights_manifest.json"
    //     )
    //   );
    // }
    // handle HTTP request normally

    return new Response(await Bun.file("./public/index.html").text(), {
      headers: {
        "Content-Type": "text/html",
      },
    });
  },
  websocket: {
    // this is called when a message is received
    async message(ws, message) {
      const buf = Buffer.from(message);
      // await Bun.write("./hello.jpg", buf);

      server.publish(
        "cam-stream",
        JSON.stringify({
          image: `data:image/jpeg;base64,${buf.toString("base64")}`,
        }),
        true
      );
    },
    async open(ws) {
      console.log(`[new connection] isCam : ${ws.data.isCam}`);
      if (!ws.data.isCam) {
        ws.subscribe("cam-stream");
      }
    },
  },
});

console.log(`Listening on ${server.hostname}:${server.port}`);
