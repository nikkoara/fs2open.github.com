#pragma once


#include "AbstractDialogModel.h"
namespace fso {
namespace fred {
namespace dialogs {


class ObjectOrientEditorDialogModel: public AbstractDialogModel {
 public:
	struct ObjectEntry {
		std::string name;
		int objIndex = -1;

		ObjectEntry(const std::string& name, int objIndex);
	};

	enum class PointToMode {
		Object,
		Location
	};

 private:
	void initializeData();

	int _selectedObjectNum = -1;
	bool _point_to = false;

	vec3d _position;
	vec3d _location;

	bool _enabled = true;

	int total = 0;

	std::vector<ObjectEntry> _entries;

	PointToMode _pointMode = PointToMode::Object;

	void update_object(object* ptr);
 public:
	ObjectOrientEditorDialogModel(QObject* parent, EditorViewport* viewport);

	bool apply() override;

	void reject() override;

	int getObjectIndex() const;
	bool isPointTo() const;
	const vec3d& getPosition() const;
	const vec3d& getLocation() const;
	bool isEnabled() const;
	const std::vector<ObjectEntry>& getEntries() const;
	PointToMode getPointMode() const;

	void setSelectedObjectNum(int selectedObjectNum);
	void setPointTo(bool point_to);
	void setPosition(const vec3d& position);
	void setLocation(const vec3d& location);
	void setPointMode(PointToMode pointMode);

	bool query_modified();
};

}
}
}

