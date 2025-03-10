# HTML to PNG Converter Service

A high-performance microservice that converts HTML content to PNG images using the Ultralight render engine. This service provides two distinct APIs for converting HTML content with different requirements.

## Features

- High-quality HTML rendering with Ultralight engine
- Support for local image embedding
- Customizable output dimensions
- Fast and efficient conversion
- Docker support for easy deployment

## Prerequisites

- Docker
- Docker Compose

## Installation

1. Clone the repository:

```bash
git clone <repository-url>
cd html-to-png
```

2. Build the Docker image:

```bash
docker-compose build
```

3. Start the service:

```bash
docker compose up
```

The service will be available at `http://localhost:3000`.

## API Endpoints

### 1. Simple HTML to PNG Converter

Converts plain HTML content to PNG image.

**Endpoint:** `POST /api/render-html-to-png`

**Request Body (JSON):**

```json
{
  "html": "<html>...</html>",
  "width": 1280, // optional, default: 1280
  "height": 720 // optional, default: 720
}
```

**Response:**

- Content-Type: `image/png`
- Body: PNG image data

### 2. HTML with Local Images to PNG Converter

Converts HTML content with local image references to PNG image.

**Endpoint:** `POST /api/render-html-with-images-to-png`

**Request Format:**

- Content-Type: `multipart/form-data`

**Form Fields:**

- `html`: HTML content (required)
- `width`: Output width in pixels (optional, default: 1280)
- `height`: Output height in pixels (optional, default: 720)
- `images`: Image files (optional, multiple files allowed)

**Example HTML with Local Images:**

```html
<div style="background-image: url('local://background.jpg');">
  <h1>Hello World</h1>
  <img src="local://logo.png" />
</div>
```

**Response:**

- Content-Type: `image/png`
- Body: PNG image data

**Notes:**

- Local image references in HTML should use the `local://` protocol followed by the filename
- Image filenames in the HTML must match the uploaded file names
- For optimal performance, pre-optimize high-resolution images

## Development

The service is built using:

- Node.js for the API server
- Ultralight for HTML rendering
- Docker for containerization
