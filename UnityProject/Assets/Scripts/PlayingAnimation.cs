using DG.Tweening;
using System.Linq;
using TMPro;
using UnityEngine;

public class PlayingAnimation : MonoBehaviour
{
    [SerializeField]
    private TextMeshProUGUI[] _animationTexts;


    void Start()
    {
        foreach (var (text, i) in _animationTexts.Select((x, i) => (x, i)))
        {
            DOVirtual.DelayedCall(0.05f * i, () =>
            {
                DOTween.Sequence()
                  .Append(text.DOFade(0f, 0.3f).SetEase(Ease.OutCubic))
                  .AppendInterval(0.1f)
                  .Append(text.DOFade(1f, 0.3f).SetEase(Ease.InCubic))
                  .AppendInterval(3f)
                  .SetLoops(-1);
            });
        }
    }
}
