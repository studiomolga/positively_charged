import oscP5.*;

OscP5 oscP5;

PFont font;
PGraphics pg;
float Xunit;
float XunitMid;
float yUnit;
Leak leakage_0; Leak leakage_1; Leak leakage_2; Leak leakage_3; Leak leakage_4; Leak leakage_5; Leak leakage_6; Leak leakage_7; Leak leakage_8;
ArrayList<Bezier> heartbeat_0; ArrayList<Bezier> heartbeat_1; ArrayList<Bezier> heartbeat_2; ArrayList<Bezier> heartbeat_3; ArrayList<Bezier> heartbeat_4;
ArrayList<Bezier> heartbeat_5; ArrayList<Bezier> heartbeat_6; ArrayList<Bezier> heartbeat_7; ArrayList<Bezier> heartbeat_8;
 float a, b;
float startTime; 
float hitTime_0, hitTime_1, hitTime_2, hitTime_3, hitTime_4, hitTime_5, hitTime_6, hitTime_7, hitTime_8 ;
float currTime_0, currTime_1, currTime_2, currTime_3, currTime_4, currTime_5, currTime_6, currTime_7, currTime_8 ;
int totalHB;
int totalWatt;

void setup() {
  fullScreen(P2D);
  noCursor();
  //size(800, 600, P2D);
  noStroke();
  
  oscP5 = new OscP5(this,7000);
  oscP5.plug(this, "oscdata", "/data");
  oscP5.plug(this, "used", "/used");
  
  totalHB = 0;
  totalWatt = 0;

  heartbeat_0 = new ArrayList<Bezier>(); heartbeat_1 = new ArrayList<Bezier>(); heartbeat_2 = new ArrayList<Bezier>(); heartbeat_3 = new ArrayList<Bezier>(); heartbeat_4 = new ArrayList<Bezier>();
  heartbeat_5 = new ArrayList<Bezier>(); heartbeat_6 = new ArrayList<Bezier>(); heartbeat_7 = new ArrayList<Bezier>(); heartbeat_8 = new ArrayList<Bezier>();
  
  //heartbeat.add(new Bezier(10, height/2, random(25,45), height/2-random(10,280), random(45,60), height/2-random(10,280), 80, height/2, 0.6, 80));

  hitTime_0 = 2750; 
  hitTime_1 = 1000; 
  startTime = millis();

  Xunit = width/10;
  XunitMid = width/2;
  yUnit = height/10;
  pg = createGraphics(width, height, P2D);
  
  leakage_0 = new Leak(Xunit*3, height/2, random(Xunit*3+10, Xunit*3+20), height/2, random(XunitMid+Xunit-10, XunitMid+Xunit-20), height/2, XunitMid+Xunit, height/2);
  leakage_1 = new Leak(Xunit, height/2, random(Xunit+10, Xunit+20), height/2, random(XunitMid-Xunit*3, XunitMid-Xunit*3+20), height/2, XunitMid-Xunit*3, height/2);
  leakage_2 = new Leak(Xunit*2, height/2, random(Xunit*2+10, Xunit*2+30), height/2, random(XunitMid-Xunit*2, XunitMid-Xunit*2+30), height/2, XunitMid-Xunit*2, height/2);
  leakage_3 = new Leak(Xunit*3+20, height/2, random(Xunit*3+30, Xunit*3+40), height/2, random(XunitMid-Xunit-70, XunitMid-Xunit-60), height/2, XunitMid-Xunit-60, height/2);
  //leakage_4 = new Leak(Xunit*4, height/2, random(Xunit*4+10, Xunit*4+20), height/2, random(XunitMid+Xunit-10, XunitMid+Xunit-20), height/2, XunitMid+Xunit, height/2);
  //leakage_5 = new Leak(Xunit, height/2, random(Xunit+10, Xunit+20), height/2, random(XunitMid-Xunit*3, XunitMid-Xunit*3+20), height/2, XunitMid-Xunit*3, height/2);
  //leakage_6 = new Leak(Xunit*4, height/2, random(Xunit*4+10, Xunit*4+20), height/2, random(XunitMid+Xunit-10, XunitMid+Xunit-20), height/2, XunitMid+Xunit, height/2);
  //leakage_7 = new Leak(Xunit, height/2, random(Xunit+10, Xunit+20), height/2, random(XunitMid-Xunit*3, XunitMid-Xunit*3+20), height/2, XunitMid-Xunit*3, height/2);
  //leakage_8 = new Leak(Xunit*4, height/2, random(Xunit*4+10, Xunit*4+20), height/2, random(XunitMid+Xunit-10, XunitMid+Xunit-20), height/2, XunitMid+Xunit, height/2);
  //leakage_9 = new Leak(Xunit, height/2, random(Xunit+10, Xunit+20), height/2, random(XunitMid-Xunit*3, XunitMid-Xunit*3+20), height/2, XunitMid-Xunit*3, height/2);
  
  font = loadFont("Avenir-Light-14.vlw"); 
  textFont(font, 14);
}

void draw() {
  background(0);
  fill (160, 205, 205, 155);
  text("Taipei city consumes:", 30, 80);
  fill (160, 255, 255, 255);
  text("26000 megawatts per hour ", 190, 80);
  fill (160, 205, 205, 155);
  text("Human heart creates:", 30, 100);
  fill (160, 255, 255, 255);
  text("1 - 5 watts ", 190, 100);

fill (255, 85, 205, 155);
  text("Number of heartbeats from the opening of the exhibition:", width - 500, 80);
 fill (235, 55, 185, 255);
  text(totalHB + " beats", width- 120, 80);

  fill (255, 85, 205, 155);
  text("Amount of watts from all hearbeats:", width - 500, 100);
  fill (235, 55, 185, 255);
  text(totalWatt + " watts", width- 120, 100);

// heartbeat 0

  for (int i = heartbeat_0.size()-1; i >= 0; i--) { 
    Bezier bezier = heartbeat_0.get(i);
    bezier.display();
    if (bezier.finished()) {
      heartbeat_0.remove(i);
    }
  }  
  
//for (int i = heartbeat_1.size()-1; i >= 0; i--) { 
//    Bezier bezier = heartbeat_1.get(i);
//    bezier.display();
//    if (bezier.finished()) {
//      heartbeat_1.remove(i);
//    }
//  }
 
//for (int i = heartbeat_2.size()-1; i >= 0; i--) { 
//    Bezier bezier = heartbeat_2.get(i);
//    bezier.display();
//    if (bezier.finished()) {
//      heartbeat_2.remove(i);
//    }
//  }

//  for (int i = heartbeat_3.size()-1; i >= 0; i--) { 
//    Bezier bezier = heartbeat_3.get(i);
//    bezier.display();
//    if (bezier.finished()) {
//      heartbeat_3.remove(i);
//    }
//  }
  
//  for (int i = heartbeat_4.size()-1; i >= 0; i--) { 
//    Bezier bezier = heartbeat_4.get(i);
//    bezier.display();
//    if (bezier.finished()) {
//      heartbeat_4.remove(i);
//    }
//  }
  
//  for (int i = heartbeat_5.size()-1; i >= 0; i--) { 
//    Bezier bezier = heartbeat_5.get(i);
//    bezier.display();
//    if (bezier.finished()) {
//      heartbeat_5.remove(i);
//    }
//  }
  
//  for (int i = heartbeat_6.size()-1; i >= 0; i--) { 
//    Bezier bezier = heartbeat_6.get(i);
//    bezier.display();
//    if (bezier.finished()) {
//      heartbeat_6.remove(i);
//    }
//  }
  
//  for (int i = heartbeat_7.size()-1; i >= 0; i--) { 
//    Bezier bezier = heartbeat_7.get(i);
//    bezier.display();
//    if (bezier.finished()) {
//      heartbeat_7.remove(i);
//    }
//  }
  
//  for (int i = heartbeat_8.size()-1; i >= 0; i--) { 
//    Bezier bezier = heartbeat_8.get(i);
//    bezier.display();
//    if (bezier.finished()) {
//      heartbeat_8.remove(i);
//    }
//  }

  currTime_0 = millis() - startTime;
  if ( currTime_0 >= hitTime_0 )
  {
    //println("adding new bezier");
    //println(hitTime_0);
    startTime = millis();
    heartbeat_0.add(new Bezier(Xunit*3-a, height/2, random(Xunit*3+10, Xunit*3+20), 
    height/2-random(0, yUnit*4), random(XunitMid+Xunit-10, XunitMid+Xunit-20), height/2-random(0, yUnit*4), XunitMid+Xunit+b, height/2, 0.3, 10));

   
    //heartbeat_1.add(new Bezier(Xunit, height/2, random(Xunit+10, Xunit+20), 
    //height/2-random(0, yUnit), random(XunitMid-Xunit*3, XunitMid-Xunit*3+20), height/2-random(0, yUnit), XunitMid-Xunit*3, height/2, 0.3, 10));
    
    //heartbeat_2.add(new Bezier(Xunit*2, height/2, random(Xunit*2+10, Xunit*2+20), 
    //height/2-random(0, yUnit*3), random(XunitMid-Xunit*2-30, XunitMid-Xunit*2), height/2-random(0, yUnit*3), XunitMid-Xunit*2, height/2, 0.3, 10));
    
    //heartbeat_3.add(new Bezier(Xunit*3+20, height/2, random(Xunit*3+30, Xunit*3+40), 
    //height/2-random(0, yUnit-40), random(XunitMid-Xunit-70, XunitMid-Xunit-60), height/2-random(0, yUnit-40), XunitMid-Xunit-60, height/2, 0.3, 10));
    
    //heartbeat_4.add(new Bezier(Xunit*4-30, height/2, random(Xunit*4-20, Xunit*4-10), 
    //height/2-random(0, yUnit-40), random(XunitMid-Xunit-70, XunitMid-Xunit-60), height/2-random(0, yUnit-40), XunitMid-Xunit-60, height/2, 0.3, 10));
  }
  
  leakage_0.display(pg, 0.1, 0.03);
  image(pg, 0, 0);
  
  //// heartbeat 1
  //leakage_1.display(pg, 0.02, 0.02);
  //image(pg, 0, 0);
  
  //  // heartbeat 2
  //leakage_2.display(pg, 0.1, 0.01);
  //image(pg, 0, 0);
 
  //  // heartbeat 3
  //leakage_3.display(pg, 0.01, 0.01);
  //image(pg, 0, 0);
}

//void mousePressed() {
//  //// A new ball object is added to the ArrayList (by default to the end)
//  //heartbeat_0.add(new Bezier(Xunit, height/2, random(Xunit+10, Xunit+20), 
//  //  height/2-random(0, yUnit), random(XunitMid-Xunit*3, XunitMid-Xunit*3+20), height/2-random(0, yUnit), XunitMid-Xunit*3, height/2, 0.1, 10));
//}

public void used(float totUsers){
  hitTime_0 = 250 * (abs(totUsers - 10) + 1);
  //println(hitTime_0);
}

public void oscdata(int heartbeat, int cranktime, int fasttime, int id) {
  //println("station " + id + " send data: ");
  //println("heartbeat: " + heartbeat);
  //println("cranktime: " + cranktime);
  //println("fasttime: " + fasttime);
  totalHB += heartbeat;
  totalWatt += (heartbeat * 2.5);
}

/* incoming osc message are forwarded to the oscEvent method. */
void oscEvent(OscMessage theOscMessage) {
  if(theOscMessage.isPlugged()==false) {
  /* print the address pattern and the typetag of the received OscMessage */
  println("### received an osc message.");
  println("### addrpattern\t"+theOscMessage.addrPattern());
  println("### typetag\t"+theOscMessage.typetag());
  }
}