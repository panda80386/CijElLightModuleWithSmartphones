using DG.Tweening;
using System;
using UniRx;
using UnityEngine;
using UnityEngine.EventSystems;
using UnityEngine.UI;

namespace Panda.UI
{
    [RequireComponent(typeof(Graphic))]

    public class CommonButton : MonoBehaviour, IPointerClickHandler, IPointerDownHandler, IPointerUpHandler
    {
        /// <summary>
        /// 押したときに白くする用のテクスチャ。
        /// </summary>
        [SerializeField]
        private Image _maskImage;

        public IObservable<PointerEventData> OnClick => _onClick;
        private Subject<PointerEventData> _onClick = new Subject<PointerEventData>();

        public IObservable<PointerEventData> OnDown => _onDown;
        private Subject<PointerEventData> _onDown = new Subject<PointerEventData>();

        public IObservable<PointerEventData> OnUp => _onUp;
        private Subject<PointerEventData> _onUp = new Subject<PointerEventData>();


        public void OnPointerClick(PointerEventData eventData)
        {
            _onClick.OnNext(eventData);
        }

        public void OnPointerDown(PointerEventData eventData)
        {
            _onDown.OnNext(eventData);

            transform.DOScale(1.1f, 0.1f).SetEase(Ease.OutBack, 5f, 1);
            _maskImage.DOFade(0.3f, 0.1f).SetEase(Ease.OutCubic);
        }

        public void OnPointerUp(PointerEventData eventData)
        {
            _onUp.OnNext(eventData);
            transform.DOScale(1f, 0.1f).SetEase(Ease.OutBack, 5f);
            _maskImage.DOFade(0f, 0.1f).SetEase(Ease.OutCubic);
        }
    }
}