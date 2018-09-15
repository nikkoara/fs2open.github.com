#pragma once

#include "mission/dialogs/AbstractDialogModel.h"

namespace fso {
namespace fred {
namespace dialogs {

class FormWingDialogModel: public AbstractDialogModel {
 Q_OBJECT

 public:
	FormWingDialogModel(QObject* parent, EditorViewport* viewport);

	bool apply() override;

	void reject() override;

	const std::string& getName() const;
	void setName(const std::string& name);

 private:
	std::string _name;

	template<typename T>
	inline void modify(T& a, const T& b);
};

template<typename T>
inline void FormWingDialogModel::modify(T& a, const T& b) {
	if (a != b) {
		a = b;
		modelChanged();
	}
}

}
}
}
