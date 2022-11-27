

#include "..\SndBase.h"
#include "sliders\BaseSlider.h"


class FormAttGraph : public GUIControl
{
	enum Consts
	{
		c_segments = 50,
		c_space = 10,
		c_rightleft_free = 4,
		c_down_space = 45,
		c_borders_in_percents = 15,
		c_slider_width = 10,
		c_sliderpos0 = 25,
		c_sliderpos1 = 50,
		c_sliderpos2 = 75,
		c_sliderpos3 = 96,		
		c_ear_size = 32,
		c_edit_w = 80,
		c_edit_h = 20,
		c_title_h = sgc_header_h - 2,
		c_combo_w = sgc_header_combo_w,
		c_combo_h = sgc_header_controls_h,
		c_combo_list_w = c_combo_w,
		c_combo_list_h = sgc_header_combo_list_h,
		c_button_w = sgc_header_buttons_w,
	};

	class PointSlider : public BaseSlider
	{
	public:
		PointSlider(GUIControl * parent, const GUIRectangle & rect);

	public:
		//Установить значение 0..1
		void SetValue(float v);
		//Возвращает значение 0..1
		float GetValue();

	private:
		//Рисование
		virtual void Draw();
	};


	class EarSlider : public BaseSlider
	{
	public:
		EarSlider(GUIControl * parent, const GUIRectangle & rect);

	public:
		//Установить значение 0..1
		void SetValue(float v);
		//Возвращает значение 0..1
		float GetValue();
		//Применять ли модифицированную громкость к звуку
		bool IsActive();

	private:
		//Рисование
		virtual void Draw();
		//Обновились параметры слайдера
		virtual void OnUpdateParameters();
	};

public:
	FormAttGraph(GUIControl * parent, const GUIRectangle & rect, bool isLightVersion);
	~FormAttGraph();


public:
	//Пересчитать график
	void UpdateGraph(bool updateSoundParams);
	//Установить текущий звук, в который прописывать выбранный график
	void SetCurrentSound(const UniqId & id);
	//Установить текущую волну для настройки графика
	void SetCurrentWave(const UniqId & id);

private:
	//Рисование
	virtual void Draw();
	//Обновить состояние графика
	void _cdecl OnUpdate(GUIControl* sender);
	//Изменилась нижняя граница
	void _cdecl OnChangeMin(GUIControl* sender);
	//Изменилась верхняя граница
	void _cdecl OnChangeMax(GUIControl* sender);
	//Расчитать прямоугольник графика из общего
	GUIRectangle CalcGraphRect(const GUIRectangle & cur);
	//Прочитать параметры графика
	void ReadCurrentParams();
	//События
	void _cdecl OnChangeGraph(GUIControl* sender);
	void _cdecl OnCheckGraphName(GUIControl* sender);
	void _cdecl OnAddNewGraph(GUIControl* sender);
	void _cdecl OnRestoreSelect(GUIControl* sender);
	void _cdecl OnAttGraphAdd(GUIControl* sender);
	void _cdecl OnAttGraphRename(GUIControl* sender);
	void _cdecl OnAttGraphRenameProcess(GUIControl* sender);
	void _cdecl OnAttGraphDelete(GUIControl* sender);
	void _cdecl OnAttGraphDeleteProcess(GUIControl* sender);

private:
	//Получить текущую структуру для модификации и текущий объект для сохранения
	SoundAttGraph * GetCurrentObject(ProjectObject * & saveObject);
	//Заполнить список пресэтов
	void FillPresetsList();

private:
	//Контролеры точек
	PointSlider * p[4];
	EarSlider * ear;
	UniqId previewWaveId;
	bool earPlay;
	FormEdit * editMinDist;
	FormEdit * editMaxDist;
	GUIPoint textMinDist;
	GUIPoint textMaxDist;
	FormComboBox * graphsList;
	FormButton * buttonAdd;
	FormButton * buttonRename;
	FormButton * buttonDelete;
	ExtName title;
	bool isInitState;
	//Константы, задающие форму
	float c[4];
	float minDist;
	float maxDist;
	//Количество сегментов
	GUIPoint segments[c_segments + 2];
	//Текущий звук в который прописываем индекс графика
	ProjectSound * currentSound;
};



