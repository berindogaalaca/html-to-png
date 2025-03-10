import express, { Request, Response } from "express";
import multer from "multer";
import path from "path";
import fs from "fs";

const app = express();
const PORT = process.env.PORT || 3000;

const storage = multer.diskStorage({
  destination: function (req, file, cb) {
    const tempDir = path.join(__dirname, "../temp");
    if (!fs.existsSync(tempDir)) {
      fs.mkdirSync(tempDir, { recursive: true });
    }
    cb(null, tempDir);
  },
  filename: function (req, file, cb) {
    cb(null, file.originalname);
  },
});

const upload = multer({ storage: storage });

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

app.post(
  "/api/render-html-with-images-to-png",
  upload.array("images"),
  (req: Request, res: Response) => {
    const htmlContent = req.body.html;
    const width = parseInt(req.body.width) || 1280;
    const height = parseInt(req.body.height) || 720;
    const files = req.files as Express.Multer.File[];

    const imagePaths: { [key: string]: string } = {};
    if (files && files.length > 0) {
      files.forEach((file) => {
        imagePaths[file.originalname] = file.path;
      });
    }

    try {
      const addon = require("../build/Release/addon");
      const buffer = addon.renderHtmlToPNGWithImages(
        htmlContent,
        width,
        height,
        imagePaths
      );

      if (files && files.length > 0) {
        files.forEach((file) => {
          try {
            fs.unlinkSync(file.path);
          } catch (e) {
            console.error("Geçici dosya silinemedi:", file.path, e);
          }
        });
      }

      res.setHeader("Content-Type", "image/png");
      res.setHeader("Content-Length", buffer.length);
      res.send(buffer);
    } catch (error) {
      console.error("Render hatası:", error);
      res
        .status(500)
        .json({ error: "HTML'i PNG'ye dönüştürürken hata oluştu" });
    }
  }
);

app.listen(PORT, () => {
  console.log(`Server is running on port ${PORT}`);
});
