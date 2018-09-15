#pragma once

#include "AbstractDialogModel.h"

namespace fso {
namespace fred {
namespace dialogs {

class CustomWingNamesDialogModel : public AbstractDialogModel {
public:
	CustomWingNamesDialogModel(QObject* parent, EditorViewport* viewport);

	bool apply() override;
	void reject() override;

	void setStartingWing(std::string, int);
	void setSquadronWing(std::string, int);
	void setTvTWing(std::string, int);
	std::string getStartingWing(int);
	std::string getSquadronWing(int);
	std::string getTvTWing(int);

	bool query_modified();
private:
	void initializeData();

	template<typename T>
	void modify(T &a, T &b);

	std::string _m_starting[3];
	std::string _m_squadron[5];
	std::string _m_tvt[2];
};

template<typename T>
inline void CustomWingNamesDialogModel::modify(T & a, T & b) {
	if (a != b) {
		a = b;
		modelChanged();
	}
}

}
}
}
