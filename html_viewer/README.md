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

### Install Node.js

Download and install Node.js from [the official website](https://nodejs.org/en).

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


1. **Download and Extract the Zip Folder**: Go [here](https://drive.google.com/file/d/1FKME-I9dDG1xW4eVcQjUGCzXnOnn8rtl/view?usp=sharing) to download the zip folder containing the Deep Zoom Images. Use the tool 7zip to extract the contents.

2. **Folder Structure Example**: Here is an example of how your folder structure should look after extracting the zip folder:

![Folder Structure](../github_assets/html_viewer_file_structure.jpg)