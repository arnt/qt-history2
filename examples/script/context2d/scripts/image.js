function draw() {
  var ctx = document.getElementById('tutorial').getContext('2d');
  var img = new Image();
  //FIXME: should be:
  //img.src = 'backdrop.png';
  img.setSrc('backdrop.png');
  ctx.drawImage(img,0,0);
  ctx.beginPath();
  ctx.moveTo(30,96);
  ctx.lineTo(70,66);
  ctx.lineTo(103,76);
  ctx.lineTo(170,15);
  ctx.stroke();
}
draw();
