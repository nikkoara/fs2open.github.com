#pragma once

#include "mission/dialogs/AbstractDialogModel.h"

namespace fso {
namespace fred {
namespace dialogs {

class WaypointEditorDialogModel: public AbstractDialogModel {
 Q_OBJECT

 public:
	struct PointListElement {
		std::string name;
		int id = -1;

		PointListElement(const std::string& name, int id);
	};

	WaypointEditorDialogModel(QObject* parent, EditorViewport* viewport);

	bool apply() override;

	void reject() override;

	static const int ID_JUMP_NODE_MENU = 8000;
	static const int ID_WAYPOINT_MENU = 9000;

	const std::string& getCurrentName() const;
	int getCurrentElementId() const;
	bool isEnabled() const;
	const std::vector<WaypointEditorDialogModel::PointListElement>& getElements() const;

	void idSelected(int elementId);
	void setNameEditText(const std::string& name);
 private:
	bool showErrorDialog(const std::string& message, const std::string& title);

	void onSelectedObjectChanged(int);
	void onSelectedObjectMarkingChanged(int, bool);
	void missionChanged();

	void updateElementList();

	void initializeData();

	std::string _currentName;
	int _currentElementId = -1;
	bool _enabled = false;
	std::vector<PointListElement> _elements;

	bool bypass_errors = false;
};

}
}
}
