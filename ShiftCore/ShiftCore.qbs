import qbs	
import "../../Eks/EksBuild" as Eks;

Eks.Library {
  name: "ShiftCore"
  toRoot: "../../"

  property bool uiSupport: true

  Depends { name: "EksCore" }

  Depends {
    condition: uiSupport
    name: "EksGui"
  }
  Depends {
    condition: uiSupport
    name: "Qt.gui"
  }

  Export {
    Depends { name: "cpp" }
    Depends { name: "EksCore" }

    cpp.includePaths: [ "./include" ]
  }
}
