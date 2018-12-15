/****************************************************************************
 *
 * Copyright (c) 2011 CRI Middleware Co., Ltd.
 *
 ****************************************************************************/

/*JP
 * 本サンプルは、3Dポジショニングを利用した3Dサウンドを行います。
 *
 * CubeにCriAtomSourceが適用されており、ヘリコプターの音が設定されています。
 * また、カメラにCriAtomListenerが適用されており、オブジェクトの位置から音が
 * 再生されているように聞こえます。
 *
 * 注意)
 * 3Dサウンドを有効にするためには、3Dポジショニング設定された音声データで
 * ある必要があります。
 */
/*EN
 * This sample performs 3D sound by utilizing 3D positioning.
 *
 * The CriAtomSource is attached to the cube, which specifies helicopter's sound.
 * Also, the CriAtomListener is attached to the camera, and so it sounds as if
 * it is played at the position of the object.
 *
 * [Note]
 * To enable 3D sound, the sound data with the 3D positioning setting
 * is needed.
 */
using UnityEngine;
using System.Collections;

public class Scene_02_3DPosition : MonoBehaviour
{
	#region Variables
	public float RotateSpeed = 100.0f;
	public Vector3 centerPosition = new Vector3(0, 0, 1.5f);
	#endregion
	
	#region Functions
	void Update()
	{
		/* Rotate around a center position. */
		this.gameObject.transform.RotateAround(centerPosition, new Vector3(0, 1, 0), Time.deltaTime * RotateSpeed);
	}
	#endregion
}

/* end of file */
