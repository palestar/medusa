/**
	@file Smoother.h
	@brief This template class can be used to smoothly interpolate from a current value to 
	a target value. 

	(c)2009 Palestar Inc
	@author Richard Lyle @date 11/6/2011 1:22:37 PM
*/

#ifndef SMOOTHER_H
#define SMOOTHER_H

//---------------------------------------------------------------------------------------------------

template<typename T>
class Smoother
{
public:
	Smoother( const T & a_Value, const T & a_Velocity ) : m_Value( a_Value ), m_Velocity( a_Velocity )
	{}

   //! Update this smoother
   //! a_To = Destination value
   //! a_fSmoothTime = Time in seconds to smooth over (i.e. 0.5f = 500 ms)
   //! a_fDeltaTime = Time elapsed since the last time this function was called
	const T & Update( const T & a_To, float a_fSmoothTime, float a_fDeltaTime )
	{
		float fOmega = 2.0f / a_fSmoothTime;
		float fX = fOmega * a_fDeltaTime;
		float fExp = 1.f/(1.f+fX+0.48f*fX*fX+0.235f*fX*fX*fX);
		
		T Change = m_Value - a_To;
		T Temp = (m_Velocity + (Change * fOmega)) * a_fDeltaTime;
		m_Velocity = (m_Velocity - (Temp * fOmega)) * fExp;
		m_Value = a_To + ((Change+Temp) * fExp);
		return m_Value;
	}

	void SetValue( const T & a_Value )
	{
		m_Value = a_Value;
	}
	void SetVelocity( const T & a_Velocity )
	{
		m_Velocity = a_Velocity;
	}
	const T & GetValue() const
	{
		return m_Value;
	}
	const T & GetVelocity() const
	{
		return m_Velocity;
	}

private:
	// Data
	T			m_Velocity;
	T			m_Value;
};

#endif

//---------------------------------------------------------------------------------------------------
//EOF
