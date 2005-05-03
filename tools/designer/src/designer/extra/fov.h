#ifndef FOV_H
#define FOV_H

class OublietteLevel;

class FOV
{
protected:
	virtual bool scanCell(OublietteLevel *map, int x, int y) = 0;
	virtual void applyCell(OublietteLevel *map, int x, int y) = 0;

	double slope(double x1, double y1, double x2, double y2);
	double invSlope(double x1, double y1, double x2, double y2);

	void scanNW2N(OublietteLevel *map, int xCenter, int yCenter, int distance, int maxRadius, double startSlope, double endSlope);
	void scanNE2N(OublietteLevel *map, int xCenter, int yCenter, int distance, int maxRadius, double startSlope, double endSlope);
	void scanNW2W(OublietteLevel *map, int xCenter, int yCenter, int distance, int maxRadius, double startSlope, double endSlope);
	void scanSW2W(OublietteLevel *map, int xCenter, int yCenter, int distance, int maxRadius, double startSlope, double endSlope);
	void scanNE2E(OublietteLevel *map, int xCenter, int yCenter, int distance, int maxRadius, double startSlope, double endSlope);
	void scanSE2E(OublietteLevel *map, int xCenter, int yCenter, int distance, int maxRadius, double startSlope, double endSlope);

	void scanSW2S(OublietteLevel *map, int xCenter, int yCenter, int distance, int maxRadius, double startSlope, double endSlope);
	void scanSE2S(OublietteLevel *map, int xCenter, int yCenter, int distance, int maxRadius, double startSlope, double endSlope);
public:
	void start(OublietteLevel *map, unsigned int x, unsigned int y, int maxRadius);
};


class SIMPLEFOV : public FOV
{
private:
	bool scanCell(OublietteLevel *map, int x, int y);
	void applyCell(OublietteLevel *map, int x, int y);
};


#endif
