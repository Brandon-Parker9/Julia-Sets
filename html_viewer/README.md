## Setting Up OpenSeadragon with Node.js Server

### Install VIPS on Linux

```bash
sudo apt install libvips-tools
```

### Generate Deep Zoom Images (DZI) with VIPS

```bash
vips dzsave <input-filename> <output-name>
```

Copy the `.dzi` file and `<output-name>` folder (which contains the images) to your HTML directory.

### Install npm (Node.js Package Manager)

Download and install npm from [the official website](https://www.npmjs.com/get-npm).

### Install OpenSeadragon Package

```bash
npm install openseadragon
```

### Include OpenSeadragon in Your HTML

```html
<div id="openseadragon1" style=" width: 900px; height: 900px;"></div>
<script src="node_modules/openseadragon/build/openseadragon/openseadragon.min.js"></script>
<script type="text/javascript">
    var viewer = OpenSeadragon({
        id: "openseadragon1",
        prefixUrl: "node_modules/openseadragon/build/openseadragon/images/",
        tileSources: "mydz.dzi"
    });
</script>
```

### Run npm Server

Start your Node.js server to serve the HTML file along with the OpenSeadragon library and DZI files.

### How to use Current zip Folder Full of Deep Zoom Images