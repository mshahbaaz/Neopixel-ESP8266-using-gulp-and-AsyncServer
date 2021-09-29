const fs = require('fs');
const del = require('del');
const uglify = require('gulp-uglify');
const gzip = require('gulp-gzip');
const htmlmin = require('gulp-htmlmin');
const cleancss = require('gulp-clean-css');
const inline = require('gulp-inline');
const inlineImages = require('gulp-css-base64');
const {src, dest, series } = require('gulp');

const dataFolder = 'build/';

function buildfs_inline() {
  return src('src/index.html')
  .pipe(inline({
    base: 'src/',
    js: uglify,
    css: [cleancss, inlineImages],
    disabledTypes: ['svg']
  }))
  .pipe(htmlmin({
      collapseWhitespace: true,
      removeComments: true,
      minifyCSS: true,
      minifyJS: true
  }))
 // .pipe(inlineImages())
  .pipe(gzip()) 
  .pipe(dest(dataFolder));
}

function buildfs_embeded() {
  var source = dataFolder + 'index.html.gz';
  var destination = dataFolder + 'index.html.gz.h';

  var wstream = fs.createWriteStream(destination);
  wstream.on('error', function (err) {
      console.log(err);
  });

  var data = fs.readFileSync(source);

  wstream.write('#define index_html_gz_len ' + data.length + '\n');
  wstream.write('const uint8_t index_html_gz[] PROGMEM = {')

  for (i=0; i<data.length; i++) {
      if (i % 1000 == 0) wstream.write("\n");
      wstream.write('0x' + ('00' + data[i].toString(16)).slice(-2));
      if (i<data.length-1) wstream.write(',');
  }

    wstream.write('};')
    wstream.end();

    return del();
  }

  exports.default = series(buildfs_inline, buildfs_embeded);