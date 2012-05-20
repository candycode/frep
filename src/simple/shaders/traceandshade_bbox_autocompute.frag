//Raytracing of functional representations
//Copyright (c) Ugo Varetto

uniform float deltaStep;
uniform vec3 halfBoxSize;

///@warning boxCenter won't work with current version of optimal composite shader
/// because box centers are not passed in
uniform vec3 boxCenter;

varying vec3 objectSpaceOrigin;
varying vec3 objectSpacePosition;
const float eps = 0.00000001;

// declaration of isofun and gradient
float IsoFunction( vec3 p );
vec3 IsoGradient( vec3 p );


vec4 ComputeColor( vec3 P, vec3 n );

void main(void)
{

  //ray origin
  vec3 origin = objectSpaceOrigin;
  //ray direction
  vec3 rdir = objectSpacePosition - origin;
  vec3 dir = normalize( rdir );
 
  //ray parameter begin/end
  float tstart = length( rdir ) - deltaStep;
  if( tstart < 0. ) discard;
  float maxDistance = ( 2. * length( halfBoxSize ) );
  if( maxDistance < eps ) discard;
  float tend = tstart + maxDistance;
  int numSteps = int( ( tend - tstart ) / deltaStep );
  vec3 prev = origin + tstart * dir;
  float tprev = tstart;
  vec3 cur = prev;
  float tcur = tprev;
  float t = -1.0;
  vec3 BBIAS = vec3( 2. * deltaStep );
  for( int i = 0; i != numSteps; ++i )
  {
  
    	   
    tcur = tstart + deltaStep * float( i );
    cur = origin+ tcur * dir;
	
    if( !( all( greaterThanEqual( cur, boxCenter - halfBoxSize - BBIAS  ) ) &&
         all( lessThanEqual( cur, boxCenter + halfBoxSize + BBIAS  ) ) ) )
    {
      t = -1.;
      break;
    }
    	
    float f1 = IsoFunction( prev );
    float f2 = IsoFunction( cur  ); 
    //this makes the look solid
    //if( i == 1 && f1 < 0. && f2 < 0. )
    //{
    //  t = 0.;
    //  cur = prev;
    //  break;
    //}
    if( f1 * f2 < 0. )
    {
      cur = prev + abs(f1 /(f1 - f2)) * ( cur - prev );
      t = tcur;
      break;         
    }
    else if( abs( f2 ) < eps )
    {
      t = length( cur - prev );
      break;
    }
    else if( abs( f1 ) < eps )
    {
      cur = prev;
      t = 0.;
      break;
    }
    else prev = cur;	
  }
  if( t < 0.0 ) discard;
 
  vec4 P = gl_ModelViewMatrix * vec4( cur, 1. );
  P.xyzw /= P.w;
  vec3 N = normalize( gl_NormalMatrix * IsoGradient( cur ) );
  gl_FragColor = ComputeColor( P.xyz, N );
  float z = dot( P, gl_ProjectionMatrixTranspose[ 2 ] );
  float w = dot( P, gl_ProjectionMatrixTranspose[ 3 ] );
  gl_FragDepth = 0.5 * ( z / w + 1.0 );
}


