import express, { Request, Response } from "express";

const app = express();
const PORT = process.env.PORT || 3000;

app.use(express.json());

app.post("/api/render-html-to-png", (req: Request, res: Response) => {
  const htmlContent = `
  <html>
    <head><title>Hello Ultralight!</title></head>
    <body><h1 style="color:blue;">Hello, world!</h1></body>
  </html>
`;

  const addon = require("../build/Release/addon");
  const buffer = addon.renderHtmlToPNG(htmlContent);

  res.setHeader("Content-Type", "image/png");
  res.setHeader("Content-Length", buffer.length);
  res.send(buffer);
});

app.listen(PORT, () => {
  console.log(`Server is running on port ${PORT}`);
});
