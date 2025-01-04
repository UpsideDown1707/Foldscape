#pragma once

namespace foldscape
{
	struct double2
	{
		double x;
		double y;
	};

	struct int2
	{
		int x;
		int y;
	};

	class Navigation
	{
	protected:
		double2 m_cursor;

	public:
		virtual void DragBegin(double2 p) = 0;
		virtual void DragUpdate(double2 dp) = 0;
		inline void CursorMotion(double2 p) { m_cursor = p; }
	};
}