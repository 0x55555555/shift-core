import qbs	
import "../../Eks/EksBuild" as Eks;

Eks.Library {
  name: "ShiftCore"
  toRoot: "../../"

  property bool uiSupport: true

  Depends { name: "Qt.gui" }
  Depends { name: "EksCore" }
  Depends { name: "EksScript" }

  Depends {
    condition: uiSupport
    name: "EksGui"
  }

  Properties {
    condition: uiSupport
  }

  Export {
    Depends { name: "cpp" }
    Depends { name: "EksCore" }
    Depends { name: "EksScript" }

    cpp.includePaths: [ "./include" ]
  }

  Eks.TestDependency { }
}
