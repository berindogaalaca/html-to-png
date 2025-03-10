import express, { Request, Response } from "express";

const app = express();
const PORT = process.env.PORT || 3000;

app.use(express.json());

app.post("/api/render-html-to-png", (req: Request, res: Response) => {
  const htmlContent = req.body.html;
  const width = req.body.width || 1280;
  const height = req.body.height || 720;

  try {
    const addon = require("../build/Release/addon");
    const buffer = addon.renderHtmlToPNG(htmlContent, width, height);

    res.setHeader("Content-Type", "image/png");
    res.setHeader("Content-Length", buffer.length);
    res.send(buffer);
  } catch (error) {
    console.error("Render hatası:", error);
    res.status(500).json({ error: "HTML'i PNG'ye dönüştürürken hata oluştu" });
  }
});

app.listen(PORT, () => {
  console.log(`Server is running on port ${PORT}`);
});
