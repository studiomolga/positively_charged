class Leak {
  float xA, yA, xB, yB, posxA, posyA, posxB, posyB;
  float sw;
  float t;
  float speed;
  

  Leak (float _posxA, float _posyA, float _xA, float _yA, float _xB, float _yB, float _posxB, float _posyB) {
    xA=_xA;
    yA=_yA;
    xB=_xB;
    yB=_yB;

    posxA=_posxA;
    posyA=_posyA;
    posxB=_posxB;
    posyB=_posyB;
  }

  void display(PGraphics p, float s, float SW) {
    p.beginDraw();
    p.noFill();
    p.stroke(255, 255, 255, 15);
     sw = SW;
    p.strokeWeight(SW);
    p.bezier(posxA, posyA, xA, yA, xB, yB, posxB, posyB);
    speed = s;
   
       yB = yB + s;
  if (yB < 0) { 
   yB = height; 
  }
   yA = yA + s;
  if (yA < 0) { 
    yA = height; 
  }
    p.endDraw();
  }
}