const { createServer } = require('node:http');
const fs = require('fs');
const path = require('path');

const hostname = '127.0.0.1';
const port = 3000;

const server = createServer((req, res) => {
  const filePath = req.url === '/' ? 'index.html' : req.url.slice(1);

  // Log the requested file path for debugging
  console.log('Requested file path:', filePath);

  // Check if the requested file exists
  fs.access(filePath, fs.constants.F_OK, (err) => {
    if (err) {
      // File does not exist, respond with a 404 error
      console.error('File not found:', filePath);
      res.writeHead(404);
      return res.end('File not found');
    }

    // Set the appropriate content type based on file extension
    let contentType = 'text/html';
    if (filePath.endsWith('.js')) {
      contentType = 'text/javascript';
    }

    // Read the file and send its content as the response
    fs.readFile(filePath, (err, data) => {
      if (err) {
        console.error('Error reading file:', err);
        res.writeHead(500);
        return res.end('Error reading the file');
      }

      res.writeHead(200, { 'Content-Type': contentType });
      res.end(data);
    });
  });
});

server.listen(port, hostname, () => {
  console.log(`Server running at http://${hostname}:${port}/`);
});
