#pragma once
#include <windows.h>

class Shape
{
protected:
	long xs1 = 0, ys1 = 0, xs2 = 0, ys2 = 0;
public:
	virtual const wchar_t* GetType() const = 0;

	long GetX1() const { return xs1; }
	long GetY1() const { return ys1; }
	long GetX2() const { return xs2; }
	long GetY2() const { return ys2; }

	void Set(long x1, long y1, long x2, long y2);
	virtual void Show(HDC hdc) = 0;
	virtual ~Shape() {}
};

class PointShape : public Shape
{
public:
	void Show(HDC hdc) override;
	const wchar_t* GetType() const override
	{
		return L"Крапка";
	}
};

class LineShape : public virtual Shape
{
public:
	void Show(HDC hdc) override;
	const wchar_t* GetType() const override
	{
		return L"Лінія";
	}
};

class RectShape : public virtual Shape
{
protected:
	bool fill = true;
public:
	void Show(HDC hdc) override;
	const wchar_t* GetType() const override
	{
		return L"Прямокутник";
	}
};

class EllipseShape : public virtual Shape
{
public:
	void Show(HDC hdc) override;
	const wchar_t* GetType() const override
	{
		return L"Еліпс";
	}
};

class LineWithCirclesShape : public LineShape, public EllipseShape
{
public:
	void Show(HDC hdc) override;
	const wchar_t* GetType() const override
	{
		return L"Лінія з кружечками";
	}
};

class CubeFrameShape : public RectShape, public LineShape
{
public:
	CubeFrameShape() { fill = false; }
	void Show(HDC hdc) override;
	const wchar_t* GetType() const override
	{
		return L"Каркас куба";
	}
};
