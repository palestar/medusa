/*
	Palette.cpp
	(c)2005 Palestar, Richard Lyle
*/

#define DRAW_DLL

#include "Debug/Assert.h"
#include "Draw/Palette.h"
#include "Draw/Image.h"

#include <limits.h>
#include <memory.h>

//----------------------------------------------------------------------------

Palette::Palette()
{}

//----------------------------------------------------------------------------

int Palette::findColor( Color color ) const
{
	int		closest = INT_MAX;
	int		closest_j = -1;

	for (int j=0;j<COLORS;j++)
	{
		int distance = m_Palette[j].distance( color );
		if (distance < closest)
		{
			if (distance == 0)		// perfect match?
				return( j );
			
			closest_j = j;
			closest = distance;
		}
	}
	ASSERT(closest_j != -1 );
	return(closest_j);
}

//----------------------------------------------------------------------------

#define SRC_RGB_BITS		(8)								// source clut color bits
#define RGB_BITS			(6)								// quantitze color bits
#define RGB_LEVELS			(1 << RGB_BITS)
#define RGB_SHIFT			(SRC_RGB_BITS - RGB_BITS)

struct MCBox
{
	word	lo[3];
	word	hi[3];
	dword	elements;
};
struct RGB
{
	byte	r,g,b;
};

unsigned int g_RGBCount[RGB_LEVELS][RGB_LEVELS][RGB_LEVELS];

//----------------------------------------------------------------------------

void OtherAxes(s32 axis,s32 *a1,s32 *a2)
{
	switch(axis)
	{
		case 0:
			*a1 = 1;
			*a2 = 2;
			break;
		case 1:
			*a1 = 0;
			*a2 = 2;
			break;
		case 2:
			*a1 = 0;
			*a2 = 1;
			break;
	}
}

void Shrink(MCBox *box)
{
	for(s32 axis=0;axis < 3;axis++)
	{
		s32		a1,a2;
		OtherAxes(axis,&a1,&a2);

		bool	stop = false;

		s32		i[3];
		for(i[axis] = box->lo[axis]; i[axis] <= box->hi[axis]; i[axis]++)
		{
			for(i[a1] = box->lo[a1]; i[a1] <= box->hi[a1]; i[a1]++)
			{
				for(i[a2] = box->lo[a2]; i[a2] <= box->hi[a2]; i[a2]++)
				{
					if (g_RGBCount[i[0]][i[1]][i[2]] > 0)
					{
						stop = true;
						break;
					}
				}
				if (stop)
					break;
			}
			if (stop)
				break;
		}
		box->lo[axis] = i[axis];

		stop = false;
		for(i[axis] = box->hi[axis]; (i[axis] + 1) >= (box->lo[axis] + 1); i[axis]--)
		{
			for(i[a1] = box->hi[a1]; (i[a1] + 1) >= (box->lo[a1] + 1); i[a1]--)
			{
				for(i[a2] = box->hi[a2]; (i[a2] + 1) >= (box->lo[a2] + 1);i[a2]--)
				{
					if (g_RGBCount[i[0]][i[1]][i[2]] > 0)
					{
						stop = true;
						break;
					}
				}
				if (stop)
					break;
			}
			if (stop)
				break;
		}
		box->hi[axis] = i[axis];
	}
}

//----------------------------------------------------------------------------

void Palette::createUsingMedianCut( const Array< Image * > & images )
{
	memset(g_RGBCount,0,sizeof(g_RGBCount));

	// tracek the total number of pixels 
	int pixelCount = 0;

	// create a pixel color history
	for(int i=0;i<images.size();i++)
	{
		// get the image
		Image * pImage = images[i];
		SizeInt imageSize( pImage->size() );

		for(int f=0;f<pImage->frameCount();f++)
		{
			Color * pSrc = (Color *)pImage->frame( f );

			int pixels = imageSize.width * imageSize.height;
			pixelCount += pixels;

			while( pixels-- )
			{
				Color pixel = *pSrc;
				pSrc++;

				g_RGBCount[ pixel.m_R >> RGB_SHIFT ][pixel.m_G >> RGB_SHIFT][pixel.m_B >> RGB_SHIFT]++;
			}
		}
	}

	MCBox	boxes[256];
	int		numBoxes = 1;

	// select the optimum colors from the color frequency data
	int c = 0;
	for(;c < 3;c++)
	{
		boxes[0].lo[c] = 0;
		boxes[0].hi[c] = RGB_LEVELS - 1;
	}
	boxes[0].elements = pixelCount;
	Shrink(boxes);

	while(numBoxes < 255)
	{
		unsigned int	maxElements = 0;
		int				selectedBox = -1;

		// pick the box with the maximum number of elements that is not a single color, split that box
		for(c=0;c<numBoxes;c++)
		{
			if ((boxes[c].elements > maxElements) && 
				((boxes[c].lo[0] < boxes[c].hi[0]) ||
				(boxes[c].lo[1] < boxes[c].hi[1]) ||
				(boxes[c].lo[2] < boxes[c].hi[2])))
			{
				maxElements = boxes[c].elements;
				selectedBox = c;
			}
		}

		// no box selected, so no more colors needed
		if (selectedBox == -1)
			break;
		
		// choose the longest axis of the box to split it along
		int axis = 0;
		int axisMax = boxes[selectedBox].hi[axis] - boxes[selectedBox].lo[axis];
		for(s32 k=1;k<3;k++)
			if (axisMax < (boxes[selectedBox].hi[k] - boxes[selectedBox].lo[k]))
			{
				axisMax = boxes[selectedBox].hi[k] - boxes[selectedBox].lo[k];
				axis = k;
			}

		int targetBox = numBoxes;
		for(c=0;c < numBoxes;c++)
			if (boxes[c].elements == 0)
			{
				targetBox = c;
				break;
			}

		int i[3];
		int a1,a2;
		OtherAxes(axis,&a1,&a2);

		unsigned int elementSum = 0;
		if (boxes[selectedBox].hi[axis] != boxes[selectedBox].lo[axis])
		{
			unsigned int planeSum = 0;
			for(i[axis]=boxes[selectedBox].lo[axis];i[axis]<=boxes[selectedBox].hi[axis];i[axis]++)
			{
				planeSum = 0;

				for(i[a1]=boxes[selectedBox].lo[a1];i[a1] <= boxes[selectedBox].hi[a1];i[a1]++)
					for(i[a2]=boxes[selectedBox].lo[a2];i[a2] <= boxes[selectedBox].hi[a2];i[a2]++)
						planeSum += g_RGBCount[i[0]][i[1]][i[2]];
				
				elementSum += planeSum;
				if (elementSum > (boxes[selectedBox].elements / 2))
					break;
			}
			if (i[axis] == boxes[selectedBox].hi[axis])
			{
				i[axis]--;
				elementSum -= planeSum;
			}
		}

		for(c=0;c<3;c++)
		{
			boxes[targetBox].lo[c] = boxes[selectedBox].lo[c];
			boxes[targetBox].hi[c] = boxes[selectedBox].hi[c];
		}

		boxes[targetBox].lo[axis] = i[axis] + 1;
		boxes[targetBox].elements = boxes[selectedBox].elements - elementSum;

		boxes[selectedBox].hi[axis] = i[axis];
		boxes[selectedBox].elements = elementSum;

		Shrink(boxes + selectedBox);
		Shrink(boxes + targetBox);

		if (targetBox == numBoxes)
			numBoxes++;
	}

	// the new clut
	RGB	newPalette[ COLORS ];
	memset( newPalette, 0, sizeof(newPalette));

	// calculate colors
	for(int index=0;index < numBoxes;index++)
	{
		// calculate a weighted sum of the color values in the box
		int	rsum,gsum,bsum;

		rsum = gsum = bsum = 0;
		for(int r=boxes[index].lo[0];r<=boxes[index].hi[0];r++)
			for(int g=boxes[index].lo[1];g<=boxes[index].hi[1];g++)
				for(int b=boxes[index].lo[2];b<=boxes[index].hi[2];b++)
				{
					unsigned int cnt = g_RGBCount[r][g][b];

					rsum += r * cnt;
					gsum += g * cnt;
					bsum += b * cnt;
				}

		// pick the actual color for that box based on the weighted sum
		rsum /= boxes[index].elements;
		newPalette[index].r = rsum > 255 ? 255 : rsum;
		gsum /= boxes[index].elements;
		newPalette[index].g = gsum > 255 ? 255 : gsum;
		bsum /= boxes[index].elements;
		newPalette[index].b = bsum > 255 ? 255 : bsum;
	}

	// set the palette
	for(int j=0;j<COLORS;j++)
	{
		m_Palette[j] = Color( newPalette[j].r << RGB_SHIFT,
			newPalette[j].g << RGB_SHIFT,
			newPalette[j].b << RGB_SHIFT, 255 );
	}
}

void Palette::createUsingMedianCut( void * pPixels, int pixelCount, ColorFormat * pFormat, byte *pDst /*= NULL*/ )
{
	memset(g_RGBCount,0,sizeof(g_RGBCount));

	// create a pixel color history
	byte * pSrc = (byte *)pPixels;
	for(int i=0;i<pixelCount;i++ )
	{
		Color pixel = pFormat->getColor( pSrc );
		pSrc += pFormat->byteSize();

		g_RGBCount[ pixel.m_R >> RGB_SHIFT ][pixel.m_G >> RGB_SHIFT][pixel.m_B >> RGB_SHIFT]++;
	}

	MCBox	boxes[256];
	int		numBoxes = 1;

	// select the optimum colors from the color frequency data
	int c = 0;
	for(;c < 3;c++)
	{
		boxes[0].lo[c] = 0;
		boxes[0].hi[c] = RGB_LEVELS - 1;
	} 
	boxes[0].elements = pixelCount;
	Shrink(boxes);

	while(numBoxes < 255)
	{
		unsigned int	maxElements = 0;
		int				selectedBox = -1;

		// pick the box with the maximum number of elements that is not a single color, split that box
		for(c=0;c<numBoxes;c++)
		{
			if ((boxes[c].elements > maxElements) && 
				((boxes[c].lo[0] < boxes[c].hi[0]) ||
				(boxes[c].lo[1] < boxes[c].hi[1]) ||
				(boxes[c].lo[2] < boxes[c].hi[2])))
			{
				maxElements = boxes[c].elements;
				selectedBox = c;
			}
		}

		// no box selected, so no more colors needed
		if (selectedBox == -1)
			break;
		
		// choose the longest axis of the box to split it along
		int axis = 0;
		int axisMax = boxes[selectedBox].hi[axis] - boxes[selectedBox].lo[axis];
		for(s32 k=1;k<3;k++)
			if (axisMax < (boxes[selectedBox].hi[k] - boxes[selectedBox].lo[k]))
			{
				axisMax = boxes[selectedBox].hi[k] - boxes[selectedBox].lo[k];
				axis = k;
			}

		int targetBox = numBoxes;
		for(c=0;c < numBoxes;c++)
			if (boxes[c].elements == 0)
			{
				targetBox = c;
				break;
			}

		int i[3];
		int a1,a2;
		OtherAxes(axis,&a1,&a2);

		unsigned int elementSum = 0;
		if (boxes[selectedBox].hi[axis] != boxes[selectedBox].lo[axis])
		{
			unsigned int planeSum = 0;

			for(i[axis]=boxes[selectedBox].lo[axis];i[axis]<=boxes[selectedBox].hi[axis];i[axis]++)
			{
				planeSum = 0;

				for(i[a1]=boxes[selectedBox].lo[a1];i[a1] <= boxes[selectedBox].hi[a1];i[a1]++)
					for(i[a2]=boxes[selectedBox].lo[a2];i[a2] <= boxes[selectedBox].hi[a2];i[a2]++)
						planeSum += g_RGBCount[i[0]][i[1]][i[2]];
				
				elementSum += planeSum;
				if (elementSum > (boxes[selectedBox].elements / 2))
					break;
			}
			if (i[axis] == boxes[selectedBox].hi[axis])
			{
				i[axis]--;
				elementSum -= planeSum;
			}
		}

		for(c=0;c<3;c++)
		{
			boxes[targetBox].lo[c] = boxes[selectedBox].lo[c];
			boxes[targetBox].hi[c] = boxes[selectedBox].hi[c];
		}

		boxes[targetBox].lo[axis] = i[axis] + 1;
		boxes[targetBox].elements = boxes[selectedBox].elements - elementSum;

		boxes[selectedBox].hi[axis] = i[axis];
		boxes[selectedBox].elements = elementSum;

		Shrink(boxes + selectedBox);
		Shrink(boxes + targetBox);

		if (targetBox == numBoxes)
			numBoxes++;
	}

	// the new clut
	RGB	newPalette[ COLORS ];
	memset( newPalette, 0, sizeof(newPalette));

	// calculate colors
	for(int index=0;index < numBoxes;index++)
	{
		// calculate a weighted sum of the color values in the box
		int	rsum,gsum,bsum;

		rsum = gsum = bsum = 0;
		for(int r=boxes[index].lo[0];r<=boxes[index].hi[0];r++)
			for(int g=boxes[index].lo[1];g<=boxes[index].hi[1];g++)
				for(int b=boxes[index].lo[2];b<=boxes[index].hi[2];b++)
				{
					unsigned int cnt = g_RGBCount[r][g][b];

					rsum += r * cnt;
					gsum += g * cnt;
					bsum += b * cnt;
				}

		// pick the actual color for that box based on the weighted sum
		rsum /= boxes[index].elements;
		newPalette[index].r = rsum > 255 ? 255 : rsum;
		gsum /= boxes[index].elements;
		newPalette[index].g = gsum > 255 ? 255 : gsum;
		bsum /= boxes[index].elements;
		newPalette[index].b = bsum > 255 ? 255 : bsum;
	}

	// set the palette
	for(int j=0;j<COLORS;j++)
	{
		m_Palette[j] = Color( newPalette[j].r << RGB_SHIFT,
			newPalette[j].g << RGB_SHIFT,
			newPalette[j].b << RGB_SHIFT, 255 );
	}

	if ( pDst != NULL )
	{
		// build the mapping array
		for(int i=0;i < numBoxes;i++)
			for(int r=boxes[i].lo[0];r <= boxes[i].hi[0];r++)
				for(int g=boxes[i].lo[1];g <= boxes[i].hi[1];g++)
					for(int b=boxes[i].lo[2];b <= boxes[i].hi[2];b++)
						g_RGBCount[r][g][b] = i;

		// remap the pixel data
		byte * pSrc = (byte *)pPixels;
		for(int p=0;p<pixelCount;p++, pDst++)
		{
			Color pixel = pFormat->getColor( pSrc );
			pSrc += pFormat->byteSize();

			*pDst = g_RGBCount[ pixel.m_R >> RGB_SHIFT ][pixel.m_G >> RGB_SHIFT][pixel.m_B >> RGB_SHIFT];
		}
	}
}

//----------------------------------------------------------------------------
// EOF

