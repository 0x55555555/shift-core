
  {
  addChildren = function(parent, type, count, runOnChild)
    {
    for(var i=0; i<count; ++i)
      {
      var ent = parent.addChild(type);
      if(runOnChild)
        {
        runOnChild(ent);
        }
      }
    }

  var tier1 = function(ent) { addChildren(ent, "SEntity", 10); };

  var tier2 = function(ent) { addChildren(ent, "SEntity", 10, tier1 ); };

  var tier3 = function(ent) { addChildren(ent, "SEntity", 100, tier2 ); };

  var parent = db.addChild("SEntity");
  tier3(parent);

  var dev = io.createFile("C:\\Users\\Jorj\\Desktop\\testDb.json");
  if(dev.open("write"))
    {
    db.save("json", dev, parent, true, false);
    dev.close();
    }
  }

  {
  var parent = db.addChild("SEntity")

  var dev = io.createFile("C:\\Users\\Jorj\\Desktop\\testDb.json");
  if(dev.open("read"))
    {
    db.load("json", dev, parent);
    dev.close();
    }
  }
