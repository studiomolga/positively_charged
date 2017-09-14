//BALL CLASS
class Bezier {
  float xA, yA, xB, yB, posxA, posyA, posxB, posyB;
  float sw;
  float t;
  float speedX= random (0.1, 3);
  float speedY=random(-0.001, -0.01);
  float life = 255;

  Bezier(float _posxA, float _posyA, float _xA, float _yA, float _xB, float _yB, float _posxB, float _posyB, float _sw, float _t) {
    xA=_xA;
    yA=_yA;
    xB=_xB;
    yB=_yB;

    posxA=_posxA;
    posyA=_posyA;
    posxB=_posxB;
    posyB=_posyB;

    sw = _sw;
    t = _t;
  }
  void run() {
    display ();
    //move();
  }

  boolean finished() {
    // Balls fade out
    life--;
    if (life < 0) {
      return true;
    } else {
      return false;
    }
  }

  //void move() {
  // yA=yA+speedY;
  // yB=yB+speedY;
  // xA=xA+speedX;
  // xB=xB+speedX;
  //}
  void display() {
    //noFill();
    fill (255, 255, 255, life/25);
    stroke(255, 255, 255, life);
    strokeWeight(sw);
    bezier(posxA, posyA, xA+random(1, 10), yA-random(1, 10), xB-random(1, 10), yB-random(1, 10), posxB, posyB);
  }
}