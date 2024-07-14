"use strict";

const net = require("net");

const server = net.createServer(client => {
  console.log("client connected");

  const conn = net.createConnection(443, "deeep-storage.anybrain.gg");

  client.on("data", data => {
    console.log("Client data:", data.toString());

    conn.write(data);
  });

  client.on("error", error => {
    console.log("Client error:", error);
  });

  client.on("end", () => {
    console.log("Client disconnected");
  });

  conn.on("data", data => {
    console.log("Conn data:", data.toString());

    client.write(data);
  });

  conn.on("error", error => {
    console.log("Conn error:", error);
  });

  conn.on("end", () => {
    console.log("Conn disconnected");
  });
});

server.listen(8080, () => {
  console.log("server bound");
});
